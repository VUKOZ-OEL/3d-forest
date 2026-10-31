/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file WebServer.cpp */

// Include 3D Forest.
#include <WebApplication.hpp>
#include <WebAuthStore.hpp>
#include <WebServer.hpp>

// Include 3rd party.
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/WebSocketClient.h>
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>

// Include std.
#include <algorithm>
#include <cctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

// Include local.
#define LOG_MODULE_NAME "WebServer"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

struct WebServer::State
{
    explicit State(Options value)
        : options(std::move(value)),
          authStore(std::make_shared<WebAuthStore>(options.accountDirectory))
    {
    }
    Options options;
    std::shared_ptr<WebAuthStore> authStore;
    WebSession::Factory factory;
    std::mutex mutex;
    std::map<std::string, std::shared_ptr<WebSession>> sessions;
    bool stopping{false};
    bool started{false};

    std::shared_ptr<WebSession> findSession(const std::string &token)
    {
        std::lock_guard<std::mutex> lock(mutex);
        const auto it = sessions.find(token);
        if (it == sessions.end() || it->second->failed())
            return {};
        return it->second;
    }

    std::shared_ptr<WebSession> get(
        const std::string &token,
        const drogon::WebSocketConnectionPtr &connection)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (stopping)
            return {};
        if (!token.empty())
        {
            auto it = sessions.find(token);
            // Unknown/expired tokens never silently select another session.
            if (it == sessions.end() || it->second->failed())
                return {};
            return it->second->attach(connection) ? it->second : nullptr;
        }
        if (sessions.size() >= options.maxSessions)
            return {};
        std::string id;
        do
        {
            id = drogon::utils::getUuid();
        } while (sessions.count(id));
        auto session = std::make_shared<WebSession>(id, factory, authStore);
        sessions.emplace(id, session);
        try
        {
            session->start();
            if (!session->attach(connection))
            {
                sessions.erase(id);
                session->stop();
                return {};
            }
        }
        catch (...)
        {
            sessions.erase(id);
            throw;
        }
        return session;
    }

    void reap()
    {
        std::vector<std::shared_ptr<WebSession>> removed;
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto it = sessions.begin(); it != sessions.end();)
            {
                if (it->second->failed() ||
                    it->second->expired(WebSession::Clock::now(),
                                        options.reconnectTimeout))
                {
                    removed.push_back(it->second);
                    it = sessions.erase(it);
                }
                else
                    ++it;
            }
        }
        for (auto &session : removed)
            session->stop();
    }

    void stop()
    {
        std::map<std::string, std::shared_ptr<WebSession>> removed;
        {
            std::lock_guard<std::mutex> lock(mutex);
            stopping = true;
            removed.swap(sessions);
        }
        for (auto &[id, session] : removed)
            session->stop();
    }
};

namespace
{
struct StreamlitWebSocketBridge
{
    drogon::WebSocketConnectionPtr browser;
    drogon::WebSocketClientPtr streamlit;
    std::deque<std::pair<std::string, drogon::WebSocketMessageType>>
        pendingBrowserMessages;
    std::size_t pendingBrowserBytes{0};
};

std::string lowerHeaderName(const std::string &name)
{
    std::string lower = name;
    std::transform(lower.begin(),
                   lower.end(),
                   lower.begin(),
                   [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });
    return lower;
}

bool isHopByHopHeader(const std::string &name)
{
    const std::string lower = lowerHeaderName(name);
    return lower == "connection" || lower == "keep-alive" ||
           lower == "proxy-authenticate" || lower == "proxy-authorization" ||
           lower == "te" || lower == "trailer" ||
           lower == "transfer-encoding" || lower == "upgrade";
}

bool isContentLengthHeader(const std::string &name)
{
    static const std::string header = "content-length";
    if (name.size() != header.size())
        return false;
    for (size_t i = 0; i < header.size(); ++i)
    {
        if (static_cast<char>(
                std::tolower(static_cast<unsigned char>(name[i]))) != header[i])
            return false;
    }
    return true;
}

bool offersSubprotocol(const std::string &header, const std::string &protocol)
{
    std::size_t start = 0;
    while (start < header.size())
    {
        const std::size_t end = header.find(',', start);
        std::size_t first = start;
        std::size_t last = end == std::string::npos ? header.size() : end;
        while (first < last &&
               std::isspace(static_cast<unsigned char>(header[first])))
            ++first;
        while (last > first &&
               std::isspace(static_cast<unsigned char>(header[last - 1])))
            --last;
        if (header.compare(first, last - first, protocol) == 0)
            return true;
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return false;
}

bool isLoopbackAddress(const std::string &address)
{
    return address == "localhost" || address == "::1" ||
           address == "0:0:0:0:0:0:0:1" || address.rfind("127.", 0) == 0;
}

bool shouldSkipIncomingHeader(const std::string &name)
{
    if (isHopByHopHeader(name) || isContentLengthHeader(name))
        return true;
    const std::string lower = lowerHeaderName(name);
    return lower == "host" || lower == "origin" || lower == "accept-encoding";
}

} // namespace

class StreamlitWebSocketProxy final
    : public drogon::WebSocketController<StreamlitWebSocketProxy, false>
{
public:
    explicit StreamlitWebSocketProxy(std::shared_ptr<WebServer::State> state)
        : state_(std::move(state))
    {
    }

    WS_PATH_LIST_BEGIN
    WS_ADD_PATH_VIA_REGEX(R"(^/marteloscope/([^/]*)(/.*)?$)");
    WS_PATH_LIST_END

    void handleNewConnection(
        const drogon::HttpRequestPtr &request,
        const drogon::WebSocketConnectionPtr &connection) override
    {
        LOG_DEBUG(<< "handleNewConnection");

        if (request->getHeader("origin") != state_->options.allowedOrigin)
        {
            LOG_WARNING(
                << "Rejecting Marteloscope WebSocket: unexpected origin '"
                << request->getHeader("origin") << "' (expected '"
                << state_->options.allowedOrigin << "')");
            connection->forceClose();
            return;
        }

        const std::string path = request->getOriginalPath();
        const std::string sessionPrefix = "/marteloscope/";
        // The WebSocket route regex requires this prefix.
        const std::size_t tokenStart = sessionPrefix.size();
        const std::size_t tokenEnd = path.find('/', tokenStart);
        const std::string token =
            path.substr(tokenStart,
                        tokenEnd == std::string::npos ? std::string::npos
                                                      : tokenEnd - tokenStart);
        if (token.size() != 36)
        {
            LOG_WARNING(<< "Rejecting Marteloscope WebSocket: invalid session "
                           "token in path '"
                        << path << "'");
            connection->forceClose();
            return;
        }
        auto session = state_->findSession(token);
        if (!session)
        {
            LOG_WARNING(<< "Rejecting Marteloscope WebSocket: session not "
                           "found for token '"
                        << token << "'");
            connection->forceClose();
            return;
        }
        const int port = session->localServicePort();
        if (port < 1 || port > 65535)
        {
            LOG_WARNING(
                << "Rejecting Marteloscope WebSocket: invalid Streamlit port "
                << port);
            connection->forceClose();
            return;
        }

        auto bridge = std::make_shared<StreamlitWebSocketBridge>();
        bridge->browser = connection;
        LOG_DEBUG(
            << "Connecting Marteloscope WebSocket to Streamlit at 127.0.0.1:"
            << port << request->getOriginalPath());
        bridge->streamlit = drogon::WebSocketClient::newWebSocketClient(
            "ws://127.0.0.1:" + std::to_string(port));
        std::weak_ptr<StreamlitWebSocketBridge> weakBridge = bridge;
        bridge->streamlit->setMessageHandler(
            [weakBridge](std::string &&message,
                         const drogon::WebSocketClientPtr &,
                         const drogon::WebSocketMessageType &type)
            {
                if (auto current = weakBridge.lock();
                    current && current->browser->connected())
                {
                    LOG_DEBUG(<< "Forwarding " << message.size()
                              << " bytes from Streamlit to browser");
                    current->browser->send(std::string_view(message), type);
                }
            });
        bridge->streamlit->setConnectionClosedHandler(
            [weakBridge](const drogon::WebSocketClientPtr &)
            {
                if (auto current = weakBridge.lock();
                    current && current->browser->connected())
                {
                    LOG_WARNING("Streamlit WebSocket connection closed");
                    current->browser->forceClose();
                }
            });

        auto upstreamRequest = drogon::HttpRequest::newHttpRequest();
        upstreamRequest->setPath(request->getOriginalPath());
        upstreamRequest->setPathEncode(false);
        const std::string subprotocol =
            request->getHeader("sec-websocket-protocol");
        if (!subprotocol.empty())
            upstreamRequest->addHeader("sec-websocket-protocol", subprotocol);
        for (const auto &parameter : request->parameters())
            upstreamRequest->setParameter(parameter.first, parameter.second);
        const std::string localOrigin =
            "http://127.0.0.1:" + std::to_string(port);
        upstreamRequest->addHeader("origin", localOrigin);
        for (const auto &cookie : request->cookies())
            upstreamRequest->addCookie(cookie.first, cookie.second);
        connection->setContext(bridge);
        bridge->streamlit->connectToServer(
            upstreamRequest,
            [bridge](drogon::ReqResult result,
                     const drogon::HttpResponsePtr &response,
                     const drogon::WebSocketClientPtr &client)
            {
                if (result != drogon::ReqResult::Ok || !client ||
                    !client->getConnection() ||
                    !client->getConnection()->connected())
                {
                    LOG_WARNING(
                        << "Streamlit WebSocket connection failed; result="
                        << static_cast<int>(result) << ", response status="
                        << (response ? static_cast<int>(response->statusCode())
                                     : 0)
                        << ", response="
                        << (response ? response->getHeader("server")
                                     : std::string()));
                    if (bridge->browser->connected())
                        bridge->browser->forceClose();
                    return;
                }
                LOG_DEBUG(
                    << "Streamlit WebSocket connected; response status="
                    << (response ? static_cast<int>(response->statusCode()) : 0)
                    << ", subprotocol="
                    << (response ? response->getHeader("sec-websocket-protocol")
                                 : std::string()));
                auto upstream = client->getConnection();
                if (!bridge->pendingBrowserMessages.empty())
                {
                    LOG_DEBUG(
                        << "Forwarding "
                        << bridge->pendingBrowserMessages.size()
                        << " queued browser WebSocket messages to Streamlit");
                }
                while (!bridge->pendingBrowserMessages.empty())
                {
                    auto &pending = bridge->pendingBrowserMessages.front();
                    upstream->send(std::string_view(pending.first),
                                   pending.second);
                    bridge->pendingBrowserMessages.pop_front();
                }
                bridge->pendingBrowserBytes = 0;
                if (!bridge->browser->connected())
                {
                    client->stop();
                }
            });
    }

    void handleNewMessage(const drogon::WebSocketConnectionPtr &connection,
                          std::string &&message,
                          const drogon::WebSocketMessageType &type) override
    {
        const auto bridge = connection->getContext<StreamlitWebSocketBridge>();
        if (bridge && bridge->streamlit)
        {
            const auto upstream = bridge->streamlit->getConnection();
            if (upstream && upstream->connected())
            {
                LOG_DEBUG(<< "Forwarding " << message.size()
                          << " bytes from browser to Streamlit");
                upstream->send(std::string_view(message), type);
            }
            else if (message.size() <= 1048576 - bridge->pendingBrowserBytes)
            {
                LOG_DEBUG(
                    << "Queueing " << message.size()
                    << " browser WebSocket bytes until Streamlit connects");
                bridge->pendingBrowserBytes += message.size();
                bridge->pendingBrowserMessages.emplace_back(std::move(message),
                                                            type);
            }
            else
            {
                connection->forceClose();
            }
        }
    }

    void handleConnectionClosed(
        const drogon::WebSocketConnectionPtr &connection) override
    {
        const auto bridge = connection->getContext<StreamlitWebSocketBridge>();
        if (bridge && bridge->streamlit)
        {
            LOG_DEBUG(<< "Browser Marteloscope WebSocket closed");
            bridge->streamlit->stop();
        }
    }

private:
    std::shared_ptr<WebServer::State> state_;
};

class WebServer::Controller final
    : public drogon::WebSocketController<Controller, false>
{
public:
    explicit Controller(std::shared_ptr<State> state) : state_(std::move(state))
    {
    }
    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ui");
    WS_PATH_LIST_END

    void handleNewConnection(
        const drogon::HttpRequestPtr &request,
        const drogon::WebSocketConnectionPtr &connection) override
    {
        // Exact Origin check prevents arbitrary websites controlling a
        // localhost UI.
        if (request->getHeader("origin") != state_->options.allowedOrigin)
        {
            connection->forceClose();
            return;
        }
        auto context = std::make_shared<Context>();
        connection->setContext(context);
        connection->setPingMessage("3dforest", std::chrono::seconds(20));
        std::weak_ptr<drogon::WebSocketConnection> weak = connection;
        drogon::app().getLoop()->runAfter(10.0,
                                          [weak, context]
                                          {
                                              if (!context->ready.load())
                                                  if (auto socket = weak.lock())
                                                      socket->forceClose();
                                          });
    }

    void handleNewMessage(const drogon::WebSocketConnectionPtr &connection,
                          std::string &&text,
                          const drogon::WebSocketMessageType &type) override
    {
        if (type == drogon::WebSocketMessageType::Ping ||
            type == drogon::WebSocketMessageType::Pong ||
            type == drogon::WebSocketMessageType::Close)
            return;
        if (type != drogon::WebSocketMessageType::Text || text.size() > 1048576)
        {
            connection->forceClose();
            return;
        }
        const auto context = connection->getContext<Context>();
        if (!context)
        {
            connection->forceClose();
            return;
        }
        try
        {
            JsonCpp::CharReaderBuilder builder;
            builder["collectComments"] = false;
            builder["failIfExtra"] = true;
            builder["rejectDupKeys"] = true;
            builder["stackLimit"] = 32;
            JsonCpp::Value message;
            std::string errors;
            const std::unique_ptr<JsonCpp::CharReader> reader(
                builder.newCharReader());
            if (!reader->parse(text.data(),
                               text.data() + text.size(),
                               &message,
                               &errors) ||
                !message.isObject() || !message["type"].isString())
            {
                connection->forceClose();
                return;
            }

            if (!context->ready.load())
            {
                if (message["type"] != "hello" ||
                    !message["token"].isString() ||
                    message["token"].asString().size() > 64)
                {
                    connection->forceClose();
                    return;
                }
                const auto token = message["token"].asString();
                auto session = state_->get(token, connection);
                if (!session)
                {
                    connection->send(
                        token.empty()
                            ? R"({"type":"unavailable","message":"Server session limit reached"})"
                            : R"({"type":"expired","message":"Session expired; start a new session"})");
                    connection->shutdown();
                    return;
                }
                context->session = session;
                context->ready = true;
            }
            else
            {
                auto session = context->session.lock();
                if (!session ||
                    !session->receive(connection, std::move(message)))
                    connection->forceClose();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Web connection failed: " << e.what() << '\n';
            connection->forceClose();
        }
    }

    void handleConnectionClosed(
        const drogon::WebSocketConnectionPtr &connection) override
    {
        if (const auto context = connection->getContext<Context>())
            if (auto session = context->session.lock())
                session->detach(connection);
    }

private:
    struct Context
    {
        std::weak_ptr<WebSession>
            session; // Accessed only by this socket's I/O thread.
        std::atomic_bool ready{false}; // Also read by handshake timeout.
    };
    std::shared_ptr<State> state_;
};

WebServer::WebServer() : WebServer(Options{})
{
}
WebServer::WebServer(Options options)
    : state_(std::make_shared<State>(std::move(options)))
{
}
WebServer::~WebServer()
{
    state_->stop();
}
void WebServer::setApplicationFactory(WebSession::Factory factory)
{
    if (state_->started)
        throw std::logic_error("Set factory before exec()");
    if (!factory)
        throw std::invalid_argument("Missing application factory");
    state_->factory = std::move(factory);
}
int WebServer::exec()
{
    if (state_->started)
        throw std::logic_error("exec() can only run once");
    if (!state_->factory)
        throw std::logic_error("Set an application factory first");
    const auto &options = state_->options;
    if (!std::filesystem::is_directory(options.documentRoot))
        throw std::invalid_argument("Document root does not exist");
    if (!options.ioThreads || !options.maxSessions ||
        options.allowedOrigin.empty() || options.reconnectTimeout.count() < 1)
        throw std::invalid_argument("Invalid server options");
    if (options.tlsCertificate.empty() != options.tlsPrivateKey.empty())
        throw std::invalid_argument(
            "TLS certificate and private key must be provided together");
    if (state_->authStore->enabled() && options.tlsCertificate.empty() &&
        !isLoopbackAddress(options.address))
        throw std::invalid_argument("Web accounts require TLS when listening "
                                    "on a non-loopback address");
    state_->started = true;
    auto &http = drogon::app();
    http.setLogLevel(trantor::Logger::kWarn);
    http.setThreadNum(options.ioThreads);
    http.setDocumentRoot(options.documentRoot);
    http.setHomePage("index.html");
    if (!options.tlsCertificate.empty())
    {
        if (!http.supportSSL())
            throw std::runtime_error("This build does not include TLS support");
        http.setSSLFiles(options.tlsCertificate, options.tlsPrivateKey);
    }
    // Let the handler reject the first byte beyond its 1 MiB limit.
    http.setClientMaxWebSocketMessageSize(1048577);
    http.setMaxConnectionNum(256);
    http.addListener(options.address, options.port);
    http.registerController(std::make_shared<Controller>(state_));
    http.registerController(std::make_shared<StreamlitWebSocketProxy>(state_));
    http.registerPreSendingAdvice(
        [](const drogon::HttpRequestPtr &request,
           const drogon::HttpResponsePtr &response)
        {
            if (response->statusCode() == drogon::k101SwitchingProtocols &&
                request->getOriginalPath().rfind("/marteloscope/", 0) == 0 &&
                offersSubprotocol(request->getHeader("sec-websocket-protocol"),
                                  "streamlit"))
                response->addHeader("sec-websocket-protocol", "streamlit");
        });
    http.registerHandlerViaRegex(
        R"(^/downloads/([0-9a-fA-F-]{36})/([0-9a-fA-F-]{36})$)",
        [state = state_](
            const drogon::HttpRequestPtr &request,
            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
            std::string token,
            std::string id)
        {
            const auto origin = request->getHeader("origin");
            if ((!origin.empty() && origin != state->options.allowedOrigin) ||
                request->getHeader("sec-fetch-site") == "cross-site")
            {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k403Forbidden);
                callback(response);
                return;
            }
            auto session = state->findSession(token);
            auto download = session ? session->takeDownload(id) : nullptr;
            auto stream =
                download ? std::make_shared<std::ifstream>(download->file,
                                                           std::ios::binary)
                         : nullptr;
            if (!stream || !*stream)
            {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k404NotFound);
                callback(response);
                return;
            }
            auto response = drogon::HttpResponse::newStreamResponse(
                [download, stream](char *buffer,
                                   std::size_t length) -> std::size_t
                {
                    if (!buffer)
                    {
                        stream->close();
                        return 0;
                    }
                    stream->read(buffer, static_cast<std::streamsize>(length));
                    return static_cast<std::size_t>(stream->gcount());
                },
                download->file.filename().u8string(),
                drogon::CT_APPLICATION_OCTET_STREAM);
            response->addHeader("Cache-Control", "no-store");
            response->addHeader("X-Content-Type-Options", "nosniff");
            callback(response);
        },
        {drogon::Get});
    http.registerHandlerViaRegex(
        R"(^/marteloscope/([0-9a-fA-F-]{36})(/.*)?$)",
        [state = state_](
            const drogon::HttpRequestPtr &incoming,
            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
            std::string token,
            std::string)
        {
            const std::string origin = incoming->getHeader("origin");
            if (!origin.empty() && origin != state->options.allowedOrigin)
            {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k403Forbidden);
                callback(response);
                return;
            }

            auto session = state->findSession(token);
            const int port = session ? session->localServicePort() : 0;
            if (port < 1 || port > 65535)
            {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k404NotFound);
                callback(response);
                return;
            }

            auto outgoing = drogon::HttpRequest::newHttpRequest();
            outgoing->setMethod(incoming->method());
            std::string originalUrl = incoming->getOriginalPath();
            if (!incoming->query().empty())
                originalUrl += "?" + incoming->query();
            outgoing->setPath(std::move(originalUrl));
            outgoing->setPathEncode(false);
            outgoing->setPassThrough(true);
            for (const auto &header : incoming->headers())
            {
                if (!shouldSkipIncomingHeader(header.first))
                    outgoing->addHeader(header.first, header.second);
            }
            outgoing->addHeader("host", "127.0.0.1:" + std::to_string(port));
            outgoing->addHeader("origin",
                                "http://127.0.0.1:" + std::to_string(port));
            outgoing->addHeader("accept-encoding", "identity");
            if (!incoming->body().empty())
                outgoing->setBody(std::string(incoming->body()));
            if (!incoming->body().empty() ||
                incoming->method() == drogon::Post ||
                incoming->method() == drogon::Put ||
                incoming->method() == drogon::Patch)
            {
                outgoing->addHeader("content-length",
                                    std::to_string(incoming->body().size()));
            }

            const std::string cookiePath = "/marteloscope/" + token + "/";
            auto client = drogon::HttpClient::newHttpClient(
                "http://127.0.0.1:" + std::to_string(port));
            client->sendRequest(
                outgoing,
                [callback = std::move(callback), client, cookiePath](
                    drogon::ReqResult result,
                    const drogon::HttpResponsePtr &upstream) mutable
                {
                    auto response = drogon::HttpResponse::newHttpResponse();
                    response->addHeader("Referrer-Policy", "no-referrer");
                    if (result != drogon::ReqResult::Ok || !upstream)
                    {
                        response->setStatusCode(drogon::k502BadGateway);
                        response->setBody("Marteloscope service unavailable");
                        callback(response);
                        return;
                    }

                    response->setStatusCode(static_cast<drogon::HttpStatusCode>(
                        upstream->statusCode()));
                    response->setBody(std::string(upstream->body()));
                    const auto contentType =
                        upstream->getHeader("content-type");
                    if (!contentType.empty())
                        response->setContentTypeString(contentType);
                    for (const auto &header : upstream->headers())
                    {
                        if (isHopByHopHeader(header.first))
                            continue;
                        const std::string lowerName =
                            lowerHeaderName(header.first);
                        // Content-Length and the other generated headers must
                        // not be copied: Drogon generates them for this body.
                        if (lowerName == "content-length" ||
                            lowerName == "content-type" ||
                            lowerName == "date" || lowerName == "server" ||
                            lowerName == "set-cookie")
                            continue;
                        response->addHeader(header.first, header.second);
                    }
                    for (const auto &entry : upstream->cookies())
                    {
                        auto cookie = entry.second;
                        cookie.setPath(cookiePath);
                        response->addCookie(std::move(cookie));
                    }
                    callback(response);
                });
        });
    const std::weak_ptr<State> weak = state_;
    const auto timer =
        http.getLoop()->runEvery(5.0,
                                 [weak]
                                 {
                                     if (auto state = weak.lock())
                                         state->reap();
                                 });
    try
    {
        http.run();
    }
    catch (...)
    {
        http.getLoop()->invalidateTimer(timer);
        state_->stop();
        throw;
    }
    http.getLoop()->invalidateTimer(timer);
    state_->stop();
    return 0;
}
void WebServer::quit()
{
    drogon::app().quit();
}
