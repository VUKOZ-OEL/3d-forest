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
#include <WebServer.hpp>

// Include 3rd party.
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>

// Include std.
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

// Include local.
#define LOG_MODULE_NAME "WebServer"
#include <Log.hpp>

struct WebServer::State
{
    explicit State(Options value) : options(std::move(value)) {}
    Options options;
    WebSession::Factory factory;
    std::mutex mutex;
    std::map<std::string, std::shared_ptr<WebSession>> sessions;
    bool stopping{false};
    bool started{false};

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
        auto session = std::make_shared<WebSession>(id, factory);
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
        if (type != drogon::WebSocketMessageType::Text || text.size() > 16384)
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

        try
        {
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
    state_->started = true;
    auto &http = drogon::app();
    http.setLogLevel(trantor::Logger::kWarn);
    http.setThreadNum(options.ioThreads);
    http.setDocumentRoot(options.documentRoot);
    http.setHomePage("index.html");
    http.setClientMaxWebSocketMessageSize(16384);
    http.setMaxConnectionNum(256);
    http.addListener(options.address, options.port);
    http.registerController(std::make_shared<Controller>(state_));
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
