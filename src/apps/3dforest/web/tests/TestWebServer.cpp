/*
    Copyright 2026 VUKOZ

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

// Include 3rdparty.
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <drogon/WebSocketClient.h>
#include <drogon/utils/Utilities.h>
#include <trantor/net/EventLoopThread.h>
#include <trantor/net/TcpServer.h>
#include <trantor/utils/MsgBuffer.h>
#include <trantor/utils/Utilities.h>

// Include 3D Forest.
#include <CheckBox.hpp>
#include <ColorSwitch.hpp>
#include <ComboBox.hpp>
#include <DoubleRangeSlider.hpp>
#include <DoubleSpinBox.hpp>
#include <GroupBox.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <Plugin.hpp>
#include <ProgressBar.hpp>
#include <PushButton.hpp>
#include <RadioButton.hpp>
#include <Slider.hpp>
#include <SpinBox.hpp>
#include <Splitter.hpp>
#include <TableWidget.hpp>
#include <Test.hpp>
#include <TextEdit.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <WebApplication.hpp>
#include <WebServer.hpp>

// Include std.
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
struct LocalUpstreamConnectionState
{
    bool websocket{false};
};

class WebServerProtocolTestPlugin final : public Plugin
{
public:
    explicit WebServerProtocolTestPlugin(int localServicePort,
                                         std::atomic_bool *blockEntered,
                                         std::atomic_bool *releaseBlockedEvent)
        : localServicePort_(localServicePort),
          blockEntered_(blockEntered),
          releaseBlockedEvent_(releaseBlockedEvent)
    {
    }

    const char *name() const override { return "WebServerProtocolTest"; }

    void initialize(Application *application) override
    {
        auto *webApplication = dynamic_cast<WebApplication *>(application);
        if (!webApplication)
            throw std::invalid_argument("Expected a WebApplication");
        webApplication_ = webApplication;

        root_ = std::make_unique<Splitter>();
        root_->setName("protocol-test-root");
        add<CheckBox>("CheckBox");
        add<ColorSwitch>("ColorSwitch");
        add<ComboBox>("ComboBox");
        add<DoubleRangeSlider>("DoubleRangeSlider");
        add<DoubleSpinBox>("DoubleSpinBox");
        add<GroupBox>("GroupBox");
        add<Label>("Label");
        add<LineEdit>("LineEdit");
        add<ProgressBar>("ProgressBar");
        add<PushButton>("PushButton");
        add<PushButton>("FailSession");
        add<PushButton>("BlockSession");
        add<RadioButton>("RadioButton");
        add<Slider>("Slider");
        add<SpinBox>("SpinBox");
        add<Splitter>("NestedSplitter");
        add<TableWidget>("TableWidget");
        add<TextEdit>("TextEdit");
        add<ToolBar>("ToolBar");
        add<ToolButton>("ToolButton");
        add<TreeWidget>("TreeWidget");

        // Publish the test controls as a bottom widget, without adding a
        // navigation item that would appear in the browser sidebar.
        webApplication->showBottomWidget(root_.get());
        webApplication->openLocalWebApp(localServicePort_);
    }

    void release() override {}

private:
    template <typename T> void add(const std::string &name)
    {
        auto widget = std::make_unique<T>();
        widget->setName(name);
        if constexpr (std::is_same_v<T, PushButton>)
        {
            widget->setPrimary(name == "PushButton");
            if (name == "FailSession")
                widget->clicked.connect(
                    []
                    { throw std::runtime_error("Intentional test failure"); });
            else if (name == "BlockSession")
                widget->clicked.connect(
                    [entered = blockEntered_, release = releaseBlockedEvent_]
                    {
                        entered->store(true);
                        while (!release->load())
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(1));
                    });
            else
                widget->clicked.connect(
                    [this] { webApplication_->closeLocalWebApp(); });
        }
        root_->addWidget(widget.release());
    }

    std::unique_ptr<Splitter> root_;
    WebApplication *webApplication_{nullptr};
    int localServicePort_;
    std::atomic_bool *blockEntered_;
    std::atomic_bool *releaseBlockedEvent_;
};

class WebServerTestFixture
{
public:
    using Response = std::pair<drogon::ReqResult, drogon::HttpResponsePtr>;

    struct WebSocketResult
    {
        bool completed{false};
        bool upgraded{false};
        bool closed{false};
        std::vector<std::string> messages;
    };

    WebServerTestFixture()
        : documentRoot_(makeDocumentRoot()),
          outsideFile_(documentRoot_.parent_path() /
                       (documentRoot_.filename().string() + "-outside.txt")),
          options_(makeOptions(documentRoot_)),
          server_(options_)
    {
        localServiceLoop_ =
            std::make_unique<trantor::EventLoopThread>("WebServerTestUpstream");
        localServiceLoop_->run();
        localService_ =
            std::make_unique<trantor::TcpServer>(localServiceLoop_->getLoop(),
                                                 trantor::InetAddress(0, true),
                                                 "WebServerTestUpstream",
                                                 true,
                                                 false);
        localService_->setRecvMessageCallback(
            [this](const trantor::TcpConnectionPtr &connection,
                   trantor::MsgBuffer *buffer)
            {
                const std::string request(buffer->peek(),
                                          buffer->readableBytes());
                buffer->retrieveAll();
                if (const auto state =
                        connection->getContext<LocalUpstreamConnectionState>())
                {
                    if (request.size() >= 6 &&
                        (static_cast<unsigned char>(request[0]) & 0x0f) == 1)
                    {
                        const std::size_t payloadLength =
                            static_cast<unsigned char>(request[1]) & 0x7f;
                        if ((static_cast<unsigned char>(request[1]) & 0x80) &&
                            request.size() >= 6 + payloadLength)
                        {
                            const std::size_t maskOffset = 2;
                            const std::size_t payloadOffset = 6;
                            std::string payload =
                                request.substr(payloadOffset, payloadLength);
                            for (std::size_t i = 0; i < payload.size(); ++i)
                                payload[i] ^= request[maskOffset + i % 4];
                            {
                                std::lock_guard<std::mutex> lock(
                                    upstreamMutex_);
                                lastUpstreamWebSocketMessage_ = payload;
                            }
                            const std::string replyPayload = "upstream-ws-ok";
                            std::string frame;
                            frame.push_back(static_cast<char>(0x81));
                            frame.push_back(
                                static_cast<char>(replyPayload.size()));
                            frame += replyPayload;
                            connection->send(std::move(frame));
                        }
                    }
                    return;
                }

                std::string lowerRequest = request;
                std::transform(lowerRequest.begin(),
                               lowerRequest.end(),
                               lowerRequest.begin(),
                               [](unsigned char c)
                               { return static_cast<char>(std::tolower(c)); });
                const auto keyHeader = lowerRequest.find("sec-websocket-key:");
                if (keyHeader != std::string::npos)
                {
                    if (rejectNextWebSocketHandshake_.exchange(false))
                    {
                        connection->send("HTTP/1.1 403 Forbidden\r\n"
                                         "Content-Length: 0\r\n"
                                         "Connection: close\r\n\r\n");
                        connection->shutdown();
                        return;
                    }
                    const auto keyStart = request.find(':', keyHeader) + 1;
                    const auto keyEnd = request.find("\r\n", keyStart);
                    auto key = request.substr(keyStart, keyEnd - keyStart);
                    const auto first = key.find_first_not_of(" \t");
                    const auto last = key.find_last_not_of(" \t");
                    key = first == std::string::npos
                              ? std::string{}
                              : key.substr(first, last - first + 1);
                    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                    const auto sha1 = trantor::utils::sha1(key);
                    unsigned char acceptBytes[20];
                    std::memcpy(acceptBytes, &sha1, sizeof(acceptBytes));
                    const auto accept =
                        drogon::utils::base64Encode(acceptBytes,
                                                    sizeof(acceptBytes));
                    std::string response =
                        "HTTP/1.1 101 Switching Protocols\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Accept: " +
                        accept + "\r\n";
                    if (lowerRequest.find(
                            "sec-websocket-protocol: streamlit") !=
                        std::string::npos)
                        response += "Sec-WebSocket-Protocol: streamlit\r\n";
                    response += "\r\n";
                    connection->setContext(
                        std::make_shared<LocalUpstreamConnectionState>());
                    {
                        std::lock_guard<std::mutex> lock(upstreamMutex_);
                        lastUpstreamRequest_ = request;
                    }
                    if (delayNextWebSocketHandshake_.exchange(false))
                    {
                        connection->getLoop()->runAfter(
                            0.5,
                            [connection,
                             response = std::move(response)]() mutable
                            {
                                if (connection->connected())
                                    connection->send(std::move(response));
                            });
                    }
                    else
                        connection->send(std::move(response));
                    return;
                }
                {
                    std::lock_guard<std::mutex> lock(upstreamMutex_);
                    lastUpstreamRequest_ = request;
                }
                if (dropNextHttpResponse_.exchange(false))
                {
                    connection->shutdown();
                    return;
                }
                if (sendNextChunkedHttpResponse_.exchange(false))
                {
                    static const std::string chunkedResponse =
                        "HTTP/1.1 200 OK\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Connection: close\r\n\r\n"
                        "c\r\nupstream-ok!\r\n0\r\n\r\n";
                    connection->send(chunkedResponse);
                    connection->shutdown();
                    return;
                }
                static const std::string response =
                    "HTTP/1.1 201 Created\r\n"
                    "Content-Length: 12\r\n"
                    "Content-Type: text/plain\r\n"
                    "X-Upstream: preserved\r\n"
                    "Connection: close\r\n"
                    "Keep-Alive: timeout=5\r\n"
                    "Proxy-Authenticate: Basic realm=test\r\n"
                    "TE: trailers\r\n"
                    "Trailer: X-Checksum\r\n"
                    "Upgrade: test-protocol\r\n"
                    "Date: Sun, 06 Nov 1994 08:49:37 GMT\r\n"
                    "Server: test-upstream\r\n"
                    "Set-Cookie: session=abc; Path=/\r\n"
                    "\r\n"
                    "upstream-ok!";
                connection->send(response);
                connection->shutdown();
            });
        localService_->start();
        localServicePort_ = localService_->address().toPort();

        std::filesystem::create_directories(documentRoot_);
        std::ofstream indexFile(documentRoot_ / "index.html");
        indexFile << "web server test page";
        indexFile.close();
        std::ofstream outsideFile(outsideFile_);
        outsideFile << "outside document root";
        outsideFile.close();

        server_.setApplicationFactory(
            [this, localServicePort = localServicePort_](WebSession &session)
            {
                int sessionServicePort = localServicePort;
                if (invalidPortForNextSession_.exchange(false))
                {
                    sessionServicePort = 70000;
                    session.setLocalServicePort(sessionServicePort);
                }
                auto application = std::make_unique<WebApplication>(session);
                auto plugin = std::make_shared<WebServerProtocolTestPlugin>(
                    sessionServicePort,
                    &blockEntered_,
                    &releaseBlockedEvent_);
                // WebApplication destroys WebUiTree in its destructor body.
                // Keep the plugin's widgets alive until that teardown has
                // removed all adapters that refer to them.
                application->setInitializeHandler(
                    [plugin](WebApplication &app)
                    { plugin->initialize(&app); });
                application->init();
                return application;
            });
        serverThread_ = std::thread(
            [this]
            {
                try
                {
                    server_.exec();
                }
                catch (...)
                {
                    serverFailed_ = true;
                }
                serverFinished_ = true;
            });

        clientLoop_ =
            std::make_unique<trantor::EventLoopThread>("WebServerTestClient");
        clientLoop_->run();
        client_ = drogon::HttpClient::newHttpClient(
            "http://127.0.0.1:" + std::to_string(options_.port),
            clientLoop_->getLoop());

        for (int i = 0; i < 60 && !serverFinished_; ++i)
        {
            if (sendRequest("/").first == drogon::ReqResult::Ok)
            {
                ready_ = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (!ready_ && !serverFinished_)
            server_.quit();
        if (!ready_ && serverThread_.joinable())
            serverThread_.join();

        Test::instance()->addCleanup([this] { stop(); });
    }

    ~WebServerTestFixture() { stop(); }

    Response get(const std::string &path)
    {
        if (!ready_ || !client_)
            return {drogon::ReqResult::NetworkFailure, nullptr};
        return sendRequest(path);
    }

    Response get(const std::string &path, const std::string &origin)
    {
        if (!ready_ || !client_)
            return {drogon::ReqResult::NetworkFailure, nullptr};
        auto request = drogon::HttpRequest::newHttpRequest();
        request->setPath(path);
        request->addHeader("origin", origin);
        return client_->sendRequest(request, 1.0);
    }

    Response sendHttpRequest(
        const std::string &path,
        drogon::HttpMethod method,
        const std::string &body,
        const std::vector<std::pair<std::string, std::string>> &headers = {})
    {
        auto request = drogon::HttpRequest::newHttpRequest();
        request->setPath(path);
        request->setMethod(method);
        request->setBody(body);
        request->addHeader("origin", options_.allowedOrigin);
        for (const auto &header : headers)
            request->addHeader(header.first, header.second);
        return client_->sendRequest(request, 2.0);
    }

    std::string lastUpstreamRequest() const
    {
        std::lock_guard<std::mutex> lock(upstreamMutex_);
        return lastUpstreamRequest_;
    }

    std::string lastUpstreamWebSocketMessage() const
    {
        std::lock_guard<std::mutex> lock(upstreamMutex_);
        return lastUpstreamWebSocketMessage_;
    }

    void dropNextHttpResponse() { dropNextHttpResponse_ = true; }

    void sendNextChunkedHttpResponse() { sendNextChunkedHttpResponse_ = true; }

    void rejectNextWebSocketHandshake()
    {
        rejectNextWebSocketHandshake_ = true;
    }

    void delayNextWebSocketHandshake() { delayNextWebSocketHandshake_ = true; }

    WebSocketResult sendWebSocketMessage(
        const std::string &path,
        const std::string &origin,
        const std::string &message,
        drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text,
        std::size_t expectedMessages = 0,
        std::function<std::vector<std::string>(const JsonCpp::Value &)>
            afterSnapshot = {},
        const std::string &subprotocol = {},
        const std::string &cookie = {},
        bool sendMessageOnConnect = true,
        std::chrono::milliseconds waitTimeout = std::chrono::seconds(3))
    {
        struct SharedResult
        {
            std::mutex mutex;
            std::condition_variable condition;
            bool handshakeComplete{false};
            bool upgraded{false};
            bool closed{false};
            bool snapshotHandled{false};
            std::vector<std::string> messages;
        };
        auto result = std::make_shared<SharedResult>();
        auto webSocket =
            drogon::WebSocketClient::newWebSocketClient("127.0.0.1",
                                                        options_.port,
                                                        false,
                                                        clientLoop_->getLoop());
        webSocket->setMessageHandler(
            [result, afterSnapshot = std::move(afterSnapshot)](
                std::string &&message,
                const drogon::WebSocketClientPtr &client,
                const drogon::WebSocketMessageType &)
            {
                JsonCpp::Value parsed;
                bool handleSnapshot = false;
                {
                    std::lock_guard<std::mutex> lock(result->mutex);
                    if (!result->snapshotHandled && afterSnapshot &&
                        message.find("\"type\":\"snapshot\"") !=
                            std::string::npos)
                    {
                        result->snapshotHandled = true;
                        handleSnapshot = true;
                    }
                    result->messages.push_back(std::move(message));
                }
                if (handleSnapshot)
                {
                    const auto &snapshotText = result->messages.back();
                    JsonCpp::CharReaderBuilder builder;
                    std::string errors;
                    const std::unique_ptr<JsonCpp::CharReader> reader(
                        builder.newCharReader());
                    if (reader->parse(snapshotText.data(),
                                      snapshotText.data() + snapshotText.size(),
                                      &parsed,
                                      &errors) &&
                        parsed["revision"].isUInt64())
                    {
                        JsonCpp::Value ack(JsonCpp::objectValue);
                        ack["type"] = "ack";
                        ack["revision"] = parsed["revision"];
                        JsonCpp::StreamWriterBuilder writer;
                        writer["indentation"] = "";
                        if (const auto connection = client->getConnection())
                        {
                            connection->send(JsonCpp::writeString(writer, ack));
                            for (const auto &followup : afterSnapshot(parsed))
                                connection->send(followup);
                        }
                    }
                }
                result->condition.notify_one();
            });
        webSocket->setConnectionClosedHandler(
            [result](const drogon::WebSocketClientPtr &client)
            {
                {
                    std::lock_guard<std::mutex> lock(result->mutex);
                    result->closed = true;
                }
                client->stop();
                result->condition.notify_one();
            });

        auto request = drogon::HttpRequest::newHttpRequest();
        request->setPath(path);
        request->addHeader("origin", origin);
        if (!subprotocol.empty())
            request->addHeader("sec-websocket-protocol", subprotocol);
        if (!cookie.empty())
            request->addCookie("browser-session", "abc");
        std::weak_ptr<drogon::WebSocketClient> weakWebSocket = webSocket;
        webSocket->connectToServer(
            request,
            [result, weakWebSocket, message, type, sendMessageOnConnect](
                drogon::ReqResult requestResult,
                const drogon::HttpResponsePtr &response,
                const drogon::WebSocketClientPtr &client)
            {
                const bool upgraded =
                    requestResult == drogon::ReqResult::Ok && response &&
                    response->statusCode() == drogon::k101SwitchingProtocols;
                {
                    std::lock_guard<std::mutex> lock(result->mutex);
                    result->handshakeComplete = true;
                    result->upgraded = upgraded;
                }
                result->condition.notify_one();
                if (!upgraded)
                    client->stop();
                if (upgraded && sendMessageOnConnect)
                {
                    if (auto client = weakWebSocket.lock())
                    {
                        if (auto connection = client->getConnection())
                            connection->send(message, type);
                    }
                }
            });

        bool completed = false;
        {
            std::unique_lock<std::mutex> lock(result->mutex);
            completed = result->condition.wait_for(
                lock,
                waitTimeout,
                [&]
                {
                    return result->closed ||
                           (result->handshakeComplete && !result->upgraded) ||
                           (expectedMessages > 0 && result->upgraded &&
                            result->messages.size() >= expectedMessages);
                });
        }

        {
            std::unique_lock<std::mutex> lock(result->mutex);
            if (!result->closed)
            {
                lock.unlock();
                if (auto connection = webSocket->getConnection())
                {
                    auto stopped = std::make_shared<std::promise<void>>();
                    auto finished = stopped->get_future();
                    clientLoop_->getLoop()->runInLoop(
                        [connection, stopped]
                        {
                            if (connection->connected())
                                connection->shutdown();
                            stopped->set_value();
                        });
                    finished.wait();
                }
                lock.lock();
            }
            result->condition.wait_for(lock,
                                       std::chrono::seconds(1),
                                       [&] { return result->closed; });
            WebSocketResult output{completed,
                                   result->upgraded,
                                   result->closed,
                                   std::move(result->messages)};
            webSocketClients_.push_back(std::move(webSocket));
            return output;
        }
    }

    bool ready() const { return ready_; }
    bool serverFailed() const { return serverFailed_; }
    void resetBlockedEvent()
    {
        blockEntered_ = false;
        releaseBlockedEvent_ = false;
    }
    bool blockedEventEntered() const { return blockEntered_; }
    void releaseBlockedEvent() { releaseBlockedEvent_ = true; }
    void useInvalidPortForNextSession() { invalidPortForNextSession_ = true; }
    WebServer &server() { return server_; }
    const std::string &allowedOrigin() const { return options_.allowedOrigin; }
    const std::filesystem::path &documentRoot() const { return documentRoot_; }
    const std::filesystem::path &outsideFile() const { return outsideFile_; }

private:
    static std::filesystem::path makeDocumentRoot()
    {
        return std::filesystem::temp_directory_path() /
               ("3DForestWebServerTest" +
                std::to_string(std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    }

    static WebServer::Options makeOptions(const std::filesystem::path &root)
    {
        WebServer::Options options;
        options.address = "127.0.0.1";
        options.port = static_cast<std::uint16_t>(
            20000 +
            (std::chrono::steady_clock::now().time_since_epoch().count() %
             40000));
        options.documentRoot = root.string();
        options.allowedOrigin =
            "http://127.0.0.1:" + std::to_string(options.port);
        options.reconnectTimeout = std::chrono::seconds(1);
        return options;
    }

    Response sendRequest(const std::string &path)
    {
        auto request = drogon::HttpRequest::newHttpRequest();
        request->setPath(path);
        return client_->sendRequest(request, 0.25);
    }

    void stop()
    {
        if (stopped_)
            return;
        if (serverThread_.joinable())
        {
            if (!serverFinished_)
                server_.quit();
            serverThread_.join();
        }
        if (clientLoop_ && !webSocketClients_.empty())
        {
            auto clients =
                std::make_shared<std::vector<drogon::WebSocketClientPtr>>(
                    std::move(webSocketClients_));
            auto stopped = std::make_shared<std::promise<void>>();
            auto finished = stopped->get_future();
            clientLoop_->getLoop()->runInLoop(
                [clients, stopped]
                {
                    for (const auto &webSocket : *clients)
                        webSocket->stop();
                    clients->clear();
                    stopped->set_value();
                });
            finished.wait();
        }
        if (localService_)
        {
            localService_->stop();
            localService_.reset();
        }
        localServiceLoop_.reset();
        client_.reset();
        clientLoop_.reset();
        std::error_code error;
        std::filesystem::remove_all(documentRoot_, error);
        std::filesystem::remove(outsideFile_, error);
        stopped_ = true;
    }

    std::filesystem::path documentRoot_;
    std::filesystem::path outsideFile_;
    WebServer::Options options_;
    WebServer server_;
    std::thread serverThread_;
    std::atomic_bool serverFinished_{false};
    std::atomic_bool serverFailed_{false};
    std::atomic_bool blockEntered_{false};
    std::atomic_bool releaseBlockedEvent_{true};
    std::atomic_bool invalidPortForNextSession_{false};
    std::unique_ptr<trantor::EventLoopThread> clientLoop_;
    std::unique_ptr<trantor::EventLoopThread> localServiceLoop_;
    std::unique_ptr<trantor::TcpServer> localService_;
    int localServicePort_{0};
    mutable std::mutex upstreamMutex_;
    std::string lastUpstreamRequest_;
    std::string lastUpstreamWebSocketMessage_;
    std::atomic_bool dropNextHttpResponse_{false};
    std::atomic_bool sendNextChunkedHttpResponse_{false};
    std::atomic_bool rejectNextWebSocketHandshake_{false};
    std::atomic_bool delayNextWebSocketHandshake_{false};
    drogon::HttpClientPtr client_;
    std::vector<drogon::WebSocketClientPtr> webSocketClients_;
    bool ready_{false};
    bool stopped_{false};
};

WebServerTestFixture &webServerFixture()
{
    static WebServerTestFixture fixture;
    return fixture;
}
} // namespace

// -----------------------------------------------------------------------------
// Static file serving

/** Serves the index by requesting the document root and checking the file. */
TEST_CASE(TestWebServerServesIndexFile)
{
    auto &fixture = webServerFixture();
    auto response = fixture.get("/");

    TEST(fixture.ready());
    TEST(!fixture.serverFailed());
    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second && response.second->statusCode() == drogon::k200OK);
    TEST(response.second &&
         std::string(response.second->body()) == "web server test page");
}

/** Returns 404 for a file absent from the document root. */
TEST_CASE(TestWebServerReturnsNotFound)
{
    auto &fixture = webServerFixture();
    auto response = fixture.get("/missing.html");

    TEST(fixture.ready());
    TEST(!fixture.serverFailed());
    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second &&
         response.second->statusCode() == drogon::k404NotFound);
}

/** Rejects path traversal requests outside the document root. */
TEST_CASE(TestWebServerDoesNotServeOutsideDocumentRoot)
{
    auto &fixture = webServerFixture();
    const std::string secret = "outside document root";
    const auto outsideName = fixture.outsideFile().filename().string();
    const std::vector<std::string> paths = {"/../" + outsideName,
                                            "/%2e%2e/" + outsideName,
                                            "/%2e%2e%2f" + outsideName};

    TEST(fixture.ready());
    for (const auto &path : paths)
    {
        const auto response = fixture.get(path);
        TEST(response.first == drogon::ReqResult::Ok);
        TEST(!response.second ||
             std::string(response.second->body()).find(secret) ==
                 std::string::npos);
    }
    TEST(!fixture.serverFailed());
}

// -----------------------------------------------------------------------------
// Server configuration and lifecycle

/** Throws during startup when the configured document root does not exist. */
TEST_CASE(TestWebServerRejectsMissingDocumentRoot)
{
    WebServer::Options options;
    options.documentRoot =
        (std::filesystem::temp_directory_path() /
         ("3DForestMissingWebRoot" +
          std::to_string(
              std::chrono::steady_clock::now().time_since_epoch().count())))
            .string();
    WebServer server(options);
    server.setApplicationFactory([](WebSession &)
                                 { return std::unique_ptr<WebApplication>{}; });

    bool rejected = false;
    try
    {
        server.exec();
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }

    TEST(rejected);
}

/** Rejects invalid thread counts, session limits, origins, or timeouts. */
TEST_CASE(TestWebServerRejectsInvalidOptions)
{
    auto &fixture = webServerFixture();
    const std::vector<std::function<void(WebServer::Options &)>> invalid = {
        [](WebServer::Options &options) { options.ioThreads = 0; },
        [](WebServer::Options &options) { options.maxSessions = 0; },
        [](WebServer::Options &options) { options.allowedOrigin.clear(); },
        [](WebServer::Options &options)
        { options.reconnectTimeout = std::chrono::seconds(0); }};

    TEST(fixture.ready());
    for (const auto &makeInvalid : invalid)
    {
        auto options = WebServer::Options{};
        options.documentRoot = fixture.documentRoot().string();
        makeInvalid(options);
        WebServer server(options);
        server.setApplicationFactory(
            [](WebSession &session)
            { return std::make_unique<WebApplication>(session); });
        bool rejected = false;
        try
        {
            server.exec();
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        TEST(rejected);
    }
    TEST(!fixture.serverFailed());
}

/** Checks errors for invalid factory changes and repeated or empty startup. */
TEST_CASE(TestWebServerRejectsInvalidLifecycleCalls)
{
    auto &fixture = webServerFixture();
    TEST(fixture.ready());

    bool factoryRejected = false;
    try
    {
        fixture.server().setApplicationFactory(
            [](WebSession &) { return std::unique_ptr<WebApplication>{}; });
    }
    catch (const std::logic_error &)
    {
        factoryRejected = true;
    }
    TEST(factoryRejected);

    bool secondExecRejected = false;
    try
    {
        fixture.server().exec();
    }
    catch (const std::logic_error &)
    {
        secondExecRejected = true;
    }
    TEST(secondExecRejected);

    WebServer::Options options;
    options.documentRoot = fixture.documentRoot().string();
    WebServer noFactory(options);
    bool missingFactoryRejected = false;
    try
    {
        noFactory.exec();
    }
    catch (const std::logic_error &)
    {
        missingFactoryRejected = true;
    }
    TEST(missingFactoryRejected);

    WebServer missingCallable(options);
    bool emptyFactoryRejected = false;
    try
    {
        missingCallable.setApplicationFactory({});
    }
    catch (const std::invalid_argument &)
    {
        emptyFactoryRejected = true;
    }
    TEST(emptyFactoryRejected);
    TEST(!fixture.serverFailed());
}

// -----------------------------------------------------------------------------
// UI WebSocket and widget protocol

/** Rejects a UI WebSocket handshake with an origin outside the allowlist. */
TEST_CASE(TestWebServerRejectsWebSocketWithWrongOrigin)
{
    auto &fixture = webServerFixture();
    const auto result =
        fixture.sendWebSocketMessage("/ui", "http://attacker.example", "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Rejects a UI WebSocket handshake that omits the required Origin header. */
TEST_CASE(TestWebServerRejectsUiWebSocketWithoutOrigin)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage("/ui", "", "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Closes a UI WebSocket when the client does not send its hello in time. */
TEST_CASE(TestWebServerClosesUiWebSocketWithoutHello)
{
    auto &fixture = webServerFixture();
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     "",
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     {},
                                     {},
                                     {},
                                     false,
                                     std::chrono::seconds(12));

    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.closed);
    TEST(!fixture.serverFailed());
}

/** Closes the UI WebSocket for malformed or oversized messages. */
TEST_CASE(TestWebServerRejectsInvalidWebSocketMessages)
{
    auto &fixture = webServerFixture();
    const std::string deepJson =
        "{\"type\":\"hello\",\"token\":\"\",\"value\":" + std::string(33, '[') +
        "0" + std::string(33, ']') + "}";
    const std::string longToken =
        "{\"type\":\"hello\",\"token\":\"" + std::string(65, 'x') + "\"}";
    const std::vector<std::string> invalidMessages = {
        "{",
        "[]",
        "{\"token\":\"\"}",
        "{\"type\":3}",
        "{\"type\":\"hello\",\"token\":3}",
        "{\"type\":\"hello\",\"type\":\"hello\",\"token\":\"\"}",
        "{\"type\":\"hello\",\"token\":\"\"} trailing",
        deepJson,
        std::string(1048577, 'x'),
        longToken,
        "{\"type\":\"event\"}"};

    TEST(fixture.ready());
    for (const auto &message : invalidMessages)
    {
        const auto result =
            fixture.sendWebSocketMessage("/ui",
                                         fixture.allowedOrigin(),
                                         message);
        TEST(result.completed);
        TEST(result.upgraded);
        TEST(result.closed);
    }
    TEST(!fixture.serverFailed());
}

/** Rejects binary frames sent to the text-based UI WebSocket protocol. */
TEST_CASE(TestWebServerRejectsBinaryUiMessages)
{
    auto &fixture = webServerFixture();
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     "{}",
                                     drogon::WebSocketMessageType::Binary);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.closed);
    TEST(!fixture.serverFailed());
}

/** Keeps UI WebSocket connections open for ping and pong control frames. */
TEST_CASE(TestWebServerIgnoresUiWebSocketPingAndPong)
{
    auto &fixture = webServerFixture();
    const auto ping =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     "ping",
                                     drogon::WebSocketMessageType::Ping,
                                     0,
                                     {},
                                     {},
                                     {},
                                     true,
                                     std::chrono::milliseconds(100));
    const auto pong =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     "pong",
                                     drogon::WebSocketMessageType::Pong,
                                     0,
                                     {},
                                     {},
                                     {},
                                     true,
                                     std::chrono::milliseconds(100));

    TEST(ping.upgraded);
    TEST(pong.upgraded);
    TEST(!fixture.serverFailed());
}

/** Publishes fake-plugin widgets without adding navigation entries. */
TEST_CASE(TestWebServerPublishesTestPluginWidgetsWithoutNavigation)
{
    auto &fixture = webServerFixture();
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.closed);
    TEST(result.messages.size() >= 2);

    JsonCpp::Value welcome;
    JsonCpp::Value snapshot;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const auto parse =
        [&builder, &errors](const std::string &text, JsonCpp::Value &value)
    {
        const std::unique_ptr<JsonCpp::CharReader> reader(
            builder.newCharReader());
        errors.clear();
        return reader->parse(text.data(),
                             text.data() + text.size(),
                             &value,
                             &errors);
    };
    TEST(parse(result.messages[0], welcome));
    TEST(parse(result.messages[1], snapshot));
    TEST(welcome["type"] == "welcome");
    TEST(snapshot["type"] == "snapshot");
    TEST(snapshot["ui"]["navigation"].isArray());
    TEST(snapshot["ui"]["navigation"].empty());
    TEST(snapshot["ui"]["bottomVisible"] == true);
    for (const auto &widget : snapshot["widgets"])
        if (widget["type"] == "PushButton")
            TEST(widget["properties"]["primary"].asBool() ==
                 (widget["properties"]["name"] == "PushButton"));

    const std::vector<std::string> expectedTypes = {"CheckBox",
                                                    "ColorSwitch",
                                                    "ComboBox",
                                                    "DoubleRangeSlider",
                                                    "DoubleSpinBox",
                                                    "GroupBox",
                                                    "Label",
                                                    "LineEdit",
                                                    "ProgressBar",
                                                    "PushButton",
                                                    "RadioButton",
                                                    "Slider",
                                                    "SpinBox",
                                                    "Splitter",
                                                    "TableWidget",
                                                    "TextEdit",
                                                    "ToolBar",
                                                    "ToolButton",
                                                    "TreeWidget"};
    for (const auto &type : expectedTypes)
    {
        bool found = false;
        for (const auto &widget : snapshot["widgets"])
            if (widget["type"].asString() == type)
                found = true;
        TEST(found);
    }
    TEST(!fixture.serverFailed());
}

/** Applies a checkbox event and checks the published state patch. */
TEST_CASE(TestWebServerAcceptsWidgetEventAndPublishesPatch)
{
    auto &fixture = webServerFixture();
    const auto followup = [](const JsonCpp::Value &snapshot)
    {
        std::string checkboxId;
        for (const auto &widget : snapshot["widgets"])
            if (widget["type"] == "CheckBox")
                checkboxId = widget["id"].asString();

        JsonCpp::Value event(JsonCpp::objectValue);
        event["type"] = "event";
        event["id"] = checkboxId;
        event["event"] = "toggled";
        event["checked"] = true;
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, event)};
    };
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     3,
                                     followup);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.messages.size() >= 3);
    JsonCpp::Value snapshot;
    JsonCpp::Value patch;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const auto parse =
        [&builder, &errors](const std::string &text, JsonCpp::Value &value)
    {
        const std::unique_ptr<JsonCpp::CharReader> reader(
            builder.newCharReader());
        errors.clear();
        return reader->parse(text.data(),
                             text.data() + text.size(),
                             &value,
                             &errors);
    };
    TEST(parse(result.messages[1], snapshot));
    TEST(parse(result.messages[2], patch));
    TEST(patch["type"] == "patch");
    std::string checkboxId;
    for (const auto &widget : snapshot["widgets"])
        if (widget["type"] == "CheckBox")
            checkboxId = widget["id"].asString();
    TEST(!checkboxId.empty());
    TEST(patch["widgets"][checkboxId]["checked"] == true);
    TEST(!fixture.serverFailed());
}

/** Returns a protocol error when a widget receives an invalid event value. */
TEST_CASE(TestWebServerReportsInvalidWidgetEvent)
{
    auto &fixture = webServerFixture();
    const auto followup = [](const JsonCpp::Value &snapshot)
    {
        std::string sliderId;
        for (const auto &widget : snapshot["widgets"])
            if (widget["type"] == "Slider")
                sliderId = widget["id"].asString();

        JsonCpp::Value event(JsonCpp::objectValue);
        event["type"] = "event";
        event["id"] = sliderId;
        event["event"] = "valueChanged";
        event["value"] = 1;
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, event)};
    };
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     3,
                                     followup);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.messages.size() >= 3);
    JsonCpp::Value error;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(result.messages[2].data(),
                       result.messages[2].data() + result.messages[2].size(),
                       &error,
                       &errors));
    TEST(error["type"] == "error");
    TEST(error["message"].isString());
    TEST(!fixture.serverFailed());
}

/** Reports an error for an unpublished state revision acknowledgement. */
TEST_CASE(TestWebServerReportsInvalidStateAcknowledgement)
{
    auto &fixture = webServerFixture();
    const auto followup = [](const JsonCpp::Value &)
    {
        JsonCpp::Value ack(JsonCpp::objectValue);
        ack["type"] = "ack";
        ack["revision"] = JsonCpp::UInt64{999};
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, ack)};
    };
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     3,
                                     followup);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.messages.size() >= 3);
    JsonCpp::Value error;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(result.messages[2].data(),
                       result.messages[2].data() + result.messages[2].size(),
                       &error,
                       &errors));
    TEST(error["type"] == "error");
    TEST(error["message"] == "Invalid state acknowledgement");
    TEST(!fixture.serverFailed());
}

/** Sends a fresh full UI snapshot after a client requests resynchronization. */
TEST_CASE(TestWebServerResendsSnapshotOnResync)
{
    auto &fixture = webServerFixture();
    const auto followup = [](const JsonCpp::Value &)
    {
        JsonCpp::Value resync(JsonCpp::objectValue);
        resync["type"] = "resync";
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, resync)};
    };
    const auto result =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     3,
                                     followup);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.messages.size() >= 3);
    JsonCpp::Value originalSnapshot;
    JsonCpp::Value resyncedSnapshot;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const auto parse =
        [&builder, &errors](const std::string &text, JsonCpp::Value &value)
    {
        const std::unique_ptr<JsonCpp::CharReader> reader(
            builder.newCharReader());
        errors.clear();
        return reader->parse(text.data(),
                             text.data() + text.size(),
                             &value,
                             &errors);
    };
    TEST(parse(result.messages[1], originalSnapshot));
    TEST(parse(result.messages[2], resyncedSnapshot));
    TEST(originalSnapshot["type"] == "snapshot");
    TEST(resyncedSnapshot["type"] == "snapshot");
    TEST(resyncedSnapshot["revision"].asUInt64() >
         originalSnapshot["revision"].asUInt64());
    TEST(!fixture.serverFailed());
}

// -----------------------------------------------------------------------------
// Marteloscope HTTP proxy

/** Checks allowed, missing, and foreign proxy Origin values. */
TEST_CASE(TestWebServerChecksMarteloscopeHttpOrigin)
{
    auto &fixture = webServerFixture();
    const std::string path =
        "/marteloscope/00000000-0000-0000-0000-000000000000/";
    const auto rejected = fixture.get(path, "http://attacker.example");
    const auto allowedButUnknown = fixture.get(path, fixture.allowedOrigin());
    const auto missingOrigin = fixture.get(path);

    TEST(fixture.ready());
    TEST(rejected.first == drogon::ReqResult::Ok);
    TEST(rejected.second &&
         rejected.second->statusCode() == drogon::k403Forbidden);
    TEST(allowedButUnknown.first == drogon::ReqResult::Ok);
    TEST(allowedButUnknown.second &&
         allowedButUnknown.second->statusCode() == drogon::k404NotFound);
    TEST(missingOrigin.first == drogon::ReqResult::Ok);
    TEST(missingOrigin.second &&
         missingOrigin.second->statusCode() == drogon::k404NotFound);
    TEST(!fixture.serverFailed());
}

/** Rejects a proxy HTTP request whose Origin uses the wrong port. */
TEST_CASE(TestWebServerRejectsMarteloscopeHttpOriginWithDifferentPort)
{
    auto &fixture = webServerFixture();
    const std::string path =
        "/marteloscope/00000000-0000-0000-0000-000000000000/";
    const auto response = fixture.get(path, "http://127.0.0.1:8081");

    TEST(fixture.ready());
    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second &&
         response.second->statusCode() == drogon::k403Forbidden);
    TEST(!fixture.serverFailed());
}

/** Proxies HTTP requests and checks the filtered headers and responses. */
TEST_CASE(TestWebServerProxiesMarteloscopeHttpRequest)
{
    auto &fixture = webServerFixture();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(hello.completed);
    TEST(hello.upgraded);
    TEST(hello.messages.size() >= 2);

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(welcome["type"] == "welcome");
    TEST(!token.empty());

    const std::string proxyPath = "/marteloscope/" + token + "/api/test?q=one";
    const auto response =
        fixture.sendHttpRequest(proxyPath,
                                drogon::Post,
                                "request-body",
                                {{"X-Test-Request", "passed"},
                                 {"Connection", "keep-alive"},
                                 {"Keep-Alive", "timeout=5"},
                                 {"Proxy-Authenticate", "Basic realm=test"},
                                 {"Proxy-Authorization", "secret"},
                                 {"TE", "trailers"},
                                 {"Transfer-Encoding", "identity"},
                                 {"Trailer", "X-Checksum"},
                                 {"Upgrade", "test-protocol"},
                                 {"Accept-Encoding", "gzip"},
                                 {"X-Forwarded-Test", "preserved"}});

    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second);
    if (response.second)
    {
        TEST(response.second->statusCode() == drogon::k201Created);
        TEST(response.second->body() == "upstream-ok!");
        TEST(response.second->getHeader("x-upstream") == "preserved");
        TEST(response.second->getHeader("referrer-policy") == "no-referrer");
        TEST(response.second->getHeader("proxy-authenticate").empty());
        TEST(response.second->getHeader("te").empty());
        TEST(response.second->getHeader("trailer").empty());
        TEST(response.second->getHeader("upgrade").empty());
        TEST(response.second->getCookie("session").path() ==
             "/marteloscope/" + token + "/");
    }

    const std::string upstreamRequest = fixture.lastUpstreamRequest();
    TEST(upstreamRequest.find("/marteloscope/" + token + "/api/test?q=one") !=
         std::string::npos);
    TEST(upstreamRequest.find("request-body") != std::string::npos);
    TEST(upstreamRequest.find("x-test-request: passed") != std::string::npos);
    TEST(upstreamRequest.find("proxy-authorization") == std::string::npos);
    TEST(upstreamRequest.find("keep-alive") == std::string::npos);
    TEST(upstreamRequest.find("accept-encoding: gzip") == std::string::npos);
    TEST(upstreamRequest.find("upgrade: test-protocol") == std::string::npos);
    TEST(upstreamRequest.find("proxy-authenticate") == std::string::npos);
    TEST(upstreamRequest.find("transfer-encoding") == std::string::npos);
    TEST(upstreamRequest.find("content-length:") != std::string::npos);

    const auto getResponse =
        fixture.sendHttpRequest("/marteloscope/" + token + "/health",
                                drogon::Get,
                                "");
    TEST(getResponse.first == drogon::ReqResult::Ok);
    TEST(getResponse.second &&
         getResponse.second->statusCode() == drogon::k201Created);

    const auto putResponse =
        fixture.sendHttpRequest("/marteloscope/" + token + "/api/resource",
                                drogon::Put,
                                "");
    TEST(putResponse.first == drogon::ReqResult::Ok);
    TEST(putResponse.second &&
         putResponse.second->statusCode() == drogon::k201Created);

    const auto patchResponse =
        fixture.sendHttpRequest("/marteloscope/" + token + "/api/resource",
                                drogon::Patch,
                                "");
    TEST(patchResponse.first == drogon::ReqResult::Ok);
    TEST(patchResponse.second &&
         patchResponse.second->statusCode() == drogon::k201Created);

    const auto emptyPostResponse =
        fixture.sendHttpRequest("/marteloscope/" + token + "/api/resource",
                                drogon::Post,
                                "");
    TEST(emptyPostResponse.first == drogon::ReqResult::Ok);
    TEST(emptyPostResponse.second &&
         emptyPostResponse.second->statusCode() == drogon::k201Created);

    fixture.sendNextChunkedHttpResponse();
    const auto chunkedResponse =
        fixture.sendHttpRequest("/marteloscope/" + token + "/api/chunked",
                                drogon::Get,
                                "");
    TEST(chunkedResponse.first == drogon::ReqResult::Ok);
    TEST(chunkedResponse.second &&
         chunkedResponse.second->statusCode() == drogon::k200OK);
    TEST(chunkedResponse.second &&
         chunkedResponse.second->body() == "upstream-ok!");
    TEST(!fixture.serverFailed());
}

/** Returns HTTP 502 when the upstream drops a proxied HTTP request. */
TEST_CASE(TestWebServerReturnsBadGatewayWhenUpstreamDropsHttpResponse)
{
    auto &fixture = webServerFixture();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(hello.messages.size() >= 1);
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    fixture.dropNextHttpResponse();
    const auto response =
        fixture.sendHttpRequest("/marteloscope/" + token + "/api/failure",
                                drogon::Get,
                                "");

    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second &&
         response.second->statusCode() == drogon::k502BadGateway);
    TEST(response.second &&
         response.second->body() == "Marteloscope service unavailable");
    TEST(!fixture.serverFailed());
}

/** Rejects HTTP and WebSocket proxy requests after the local app closes. */
TEST_CASE(TestWebServerRejectsMarteloscopeProxyAfterLocalAppCloses)
{
    auto &fixture = webServerFixture();
    const auto closeLocalApp = [](const JsonCpp::Value &snapshot)
    {
        std::string buttonId;
        for (const auto &widget : snapshot["widgets"])
            if (widget["properties"]["name"] == "PushButton")
                buttonId = widget["id"].asString();

        JsonCpp::Value event(JsonCpp::objectValue);
        event["type"] = "event";
        event["id"] = buttonId;
        event["event"] = "clicked";
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, event)};
    };
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     3,
                                     closeLocalApp);

    TEST(hello.completed);
    TEST(hello.upgraded);
    TEST(hello.messages.size() >= 3);
    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    const auto response =
        fixture.get("/marteloscope/" + token + "/", fixture.allowedOrigin());
    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second &&
         response.second->statusCode() == drogon::k404NotFound);

    const auto websocket =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "{}",
                                     drogon::WebSocketMessageType::Text);
    TEST(!websocket.upgraded || websocket.closed);
    TEST(!fixture.serverFailed());
}

/** Rejects HTTP and WebSocket proxies with an out-of-range service port. */
TEST_CASE(TestWebServerRejectsProxyWithOutOfRangeServicePort)
{
    auto &fixture = webServerFixture();
    fixture.useInvalidPortForNextSession();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(hello.messages.size() >= 1);
    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    const auto http = fixture.get("/marteloscope/" + token + "/health");
    TEST(http.first == drogon::ReqResult::Ok);
    TEST(http.second && http.second->statusCode() == drogon::k404NotFound);

    const auto webSocket =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "{}");
    TEST(webSocket.completed);
    TEST(!webSocket.upgraded || webSocket.closed);
    TEST(!fixture.serverFailed());
}

/** Returns HTTP 404 for a malformed proxy route path. */
TEST_CASE(TestWebServerReturnsNotFoundForMalformedMarteloscopePath)
{
    auto &fixture = webServerFixture();
    const auto response = fixture.get("/marteloscope/not-a-session/");

    TEST(fixture.ready());
    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second &&
         response.second->statusCode() == drogon::k404NotFound);
    TEST(!fixture.serverFailed());
}

// -----------------------------------------------------------------------------
// Marteloscope WebSocket proxy

/** Rejects a Marteloscope WebSocket handshake that uses a disallowed origin. */
TEST_CASE(TestWebServerRejectsMarteloscopeWebSocketWithWrongOrigin)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/marteloscope/00000000-0000-0000-0000-000000000000/",
        "http://attacker.example",
        "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Rejects proxy WebSocket handshakes without an Origin header. */
TEST_CASE(TestWebServerRejectsMarteloscopeWebSocketWithoutOrigin)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/marteloscope/00000000-0000-0000-0000-000000000000/",
        "",
        "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Rejects a proxy WebSocket request with a token for no active session. */
TEST_CASE(TestWebServerRejectsUnknownMarteloscopeWebSocketSession)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/marteloscope/00000000-0000-0000-0000-000000000000/",
        fixture.allowedOrigin(),
        "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Rejects a Marteloscope WebSocket path with a malformed session token. */
TEST_CASE(TestWebServerRejectsMalformedMarteloscopeWebSocketToken)
{
    auto &fixture = webServerFixture();
    const auto result =
        fixture.sendWebSocketMessage("/marteloscope/not-a-session/stream",
                                     fixture.allowedOrigin(),
                                     "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

/** Proxies a browser WebSocket and checks handshake and message forwarding. */
TEST_CASE(TestWebServerConnectsMarteloscopeWebSocketProxyToUpstream)
{
    auto &fixture = webServerFixture();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(hello.completed);
    TEST(hello.upgraded);
    TEST(hello.messages.size() >= 1);

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    const auto result = fixture.sendWebSocketMessage(
        "/marteloscope/" + token + "/stream?view=chart",
        fixture.allowedOrigin(),
        "queued-before-upstream-connect",
        drogon::WebSocketMessageType::Text,
        1,
        {},
        "streamlit , ,other",
        "browser-session=abc");

    TEST(result.upgraded);
    TEST(result.messages.size() == 1);
    if (!result.messages.empty())
        TEST(result.messages[0] == "upstream-ws-ok");
    const std::string upstreamRequest = fixture.lastUpstreamRequest();
    TEST(upstreamRequest.find("/marteloscope/" + token +
                              "/stream?view=chart") != std::string::npos);
    TEST(upstreamRequest.find("sec-websocket-protocol: streamlit , ,other") !=
         std::string::npos);
    TEST(upstreamRequest.find("browser-session=abc") != std::string::npos);
    TEST(fixture.lastUpstreamWebSocketMessage() ==
         "queued-before-upstream-connect");

    const auto withoutStreamlitProtocol =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "no-subprotocol-match",
                                     drogon::WebSocketMessageType::Text,
                                     1,
                                     {},
                                     "other");
    TEST(withoutStreamlitProtocol.upgraded);
    TEST(withoutStreamlitProtocol.messages.size() == 1);
    if (!withoutStreamlitProtocol.messages.empty())
        TEST(withoutStreamlitProtocol.messages[0] == "upstream-ws-ok");

    const auto whitespaceOnlyProtocolItems =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "no-subprotocol-match",
                                     drogon::WebSocketMessageType::Text,
                                     1,
                                     {},
                                     "other , ,other");
    TEST(whitespaceOnlyProtocolItems.upgraded);
    TEST(whitespaceOnlyProtocolItems.messages.size() == 1);
    if (!whitespaceOnlyProtocolItems.messages.empty())
        TEST(whitespaceOnlyProtocolItems.messages[0] == "upstream-ws-ok");

    const auto noSubprotocolOrQueuedMessage =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "",
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     {},
                                     {},
                                     {},
                                     false,
                                     std::chrono::milliseconds(100));
    TEST(noSubprotocolOrQueuedMessage.upgraded);
    TEST(noSubprotocolOrQueuedMessage.messages.empty());

    const auto pathWithoutSuffix =
        fixture.sendWebSocketMessage("/marteloscope/" + token,
                                     fixture.allowedOrigin(),
                                     "path-without-suffix",
                                     drogon::WebSocketMessageType::Text,
                                     1);
    TEST(pathWithoutSuffix.upgraded);
    TEST(pathWithoutSuffix.messages.size() == 1);
    if (!pathWithoutSuffix.messages.empty())
        TEST(pathWithoutSuffix.messages[0] == "upstream-ws-ok");
    TEST(!fixture.serverFailed());
}

/** Closes a browser WebSocket when the upstream rejects its handshake. */
TEST_CASE(TestWebServerClosesMarteloscopeWebSocketOnUpstreamReject)
{
    auto &fixture = webServerFixture();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(hello.messages.size() >= 1);
    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    fixture.rejectNextWebSocketHandshake();
    const auto result =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     "message-before-rejected-handshake",
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     {},
                                     "streamlit");

    TEST(result.upgraded);
    TEST(result.messages.empty());
    TEST(!fixture.serverFailed());
}

/** Closes a proxy WebSocket when queued browser data exceeds one MiB. */
TEST_CASE(TestWebServerRejectsOversizedQueuedProxyMessage)
{
    auto &fixture = webServerFixture();
    const auto hello =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    TEST(hello.messages.size() >= 1);
    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(hello.messages[0].data(),
                       hello.messages[0].data() + hello.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    fixture.delayNextWebSocketHandshake();
    const auto result =
        fixture.sendWebSocketMessage("/marteloscope/" + token + "/stream",
                                     fixture.allowedOrigin(),
                                     std::string(1048577, 'x'),
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     {},
                                     {},
                                     {},
                                     true,
                                     std::chrono::seconds(3));

    TEST(result.upgraded);
    TEST(!fixture.serverFailed());
}

// -----------------------------------------------------------------------------
// UI sessions

/** Reports an expired-session error for an invalid reconnect token. */
TEST_CASE(TestWebServerRejectsExpiredSessionToken)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        R"({"type":"hello","token":"00000000-0000-0000-0000-000000000000"})",
        drogon::WebSocketMessageType::Text,
        1);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.closed);
    TEST(!result.messages.empty());
    JsonCpp::Value response;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(result.messages[0].data(),
                       result.messages[0].data() + result.messages[0].size(),
                       &response,
                       &errors));
    TEST(response["type"] == "expired");
    TEST(!fixture.serverFailed());
}

/** Reconnects a UI client with its welcome token and checks continuity. */
TEST_CASE(TestWebServerReconnectsWithSessionToken)
{
    auto &fixture = webServerFixture();
    const auto first =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);
    TEST(first.completed);
    TEST(first.upgraded);
    TEST(first.messages.size() >= 2);

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(first.messages[0].data(),
                       first.messages[0].data() + first.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(welcome["type"] == "welcome");
    TEST(!token.empty());

    const auto reconnected = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        "{\"type\":\"hello\",\"token\":\"" + token + "\"}",
        drogon::WebSocketMessageType::Text,
        2);
    TEST(reconnected.completed);
    TEST(reconnected.upgraded);
    TEST(reconnected.messages.size() >= 2);
    JsonCpp::Value reconnectWelcome;
    TEST(reader->parse(reconnected.messages[0].data(),
                       reconnected.messages[0].data() +
                           reconnected.messages[0].size(),
                       &reconnectWelcome,
                       &errors));
    TEST(reconnectWelcome["type"] == "welcome");
    TEST(reconnectWelcome["token"] == token);
    TEST(!fixture.serverFailed());
}

/** Rejects UI input and reconnects when a blocked session queue fills. */
TEST_CASE(TestWebServerRejectsMessagesWhenSessionQueueIsFull)
{
    auto &fixture = webServerFixture();
    fixture.resetBlockedEvent();
    struct ReleaseBlockedEvent
    {
        WebServerTestFixture &fixture;
        ~ReleaseBlockedEvent() { fixture.releaseBlockedEvent(); }
    } release{fixture};

    const auto blockSession = [](const JsonCpp::Value &snapshot)
    {
        std::string buttonId;
        for (const auto &widget : snapshot["widgets"])
        {
            if (widget["properties"]["name"] == "BlockSession")
                buttonId = widget["id"].asString();
        }

        JsonCpp::Value event(JsonCpp::objectValue);
        event["type"] = "event";
        event["id"] = buttonId;
        event["event"] = "clicked";
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        const auto serialized = JsonCpp::writeString(writer, event);
        return std::vector<std::string>(1100, serialized);
    };
    const auto first =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     blockSession,
                                     {},
                                     {},
                                     true,
                                     std::chrono::seconds(3));

    TEST(first.upgraded);
    TEST(fixture.blockedEventEntered());
    TEST(!first.messages.empty());

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(first.messages[0].data(),
                       first.messages[0].data() + first.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    const auto reconnect = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        "{\"type\":\"hello\",\"token\":\"" + token + "\"}",
        drogon::WebSocketMessageType::Text,
        1);
    TEST(reconnect.upgraded);
    TEST(reconnect.closed);
    TEST(!reconnect.messages.empty());
    if (!reconnect.messages.empty())
        TEST(reconnect.messages[0].find("\"type\":\"expired\"") !=
             std::string::npos);
    TEST(!fixture.serverFailed());
}

/** Removes a session after its fake application reports a receive failure. */
TEST_CASE(TestWebServerExpiresAndReapsFailedSession)
{
    auto &fixture = webServerFixture();
    const auto failSession = [](const JsonCpp::Value &snapshot)
    {
        std::string buttonId;
        for (const auto &widget : snapshot["widgets"])
            if (widget["properties"]["name"] == "FailSession")
                buttonId = widget["id"].asString();

        JsonCpp::Value event(JsonCpp::objectValue);
        event["type"] = "event";
        event["id"] = buttonId;
        event["event"] = "clicked";
        JsonCpp::StreamWriterBuilder writer;
        writer["indentation"] = "";
        return std::vector<std::string>{JsonCpp::writeString(writer, event)};
    };
    const auto first =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     0,
                                     failSession,
                                     {},
                                     {},
                                     true,
                                     std::chrono::seconds(3));

    TEST(first.upgraded);
    TEST(first.closed);
    TEST(!first.messages.empty());
    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(reader->parse(first.messages[0].data(),
                       first.messages[0].data() + first.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    const auto failedReconnect = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        "{\"type\":\"hello\",\"token\":\"" + token + "\"}",
        drogon::WebSocketMessageType::Text,
        1);
    TEST(failedReconnect.upgraded);
    TEST(failedReconnect.closed);
    TEST(!failedReconnect.messages.empty());
    if (!failedReconnect.messages.empty())
        TEST(failedReconnect.messages[0].find("\"type\":\"expired\"") !=
             std::string::npos);

    const auto failedProxy = fixture.get("/marteloscope/" + token + "/health");
    TEST(failedProxy.first == drogon::ReqResult::Ok);
    TEST(failedProxy.second &&
         failedProxy.second->statusCode() == drogon::k404NotFound);

    std::this_thread::sleep_for(std::chrono::seconds(6));
    TEST(!fixture.serverFailed());
}

/** Expires and reaps a disconnected session after its inactivity timeout. */
TEST_CASE(TestWebServerReapsExpiredDisconnectedSession)
{
    auto &fixture = webServerFixture();
    const auto first =
        fixture.sendWebSocketMessage("/ui",
                                     fixture.allowedOrigin(),
                                     R"({"type":"hello","token":""})",
                                     drogon::WebSocketMessageType::Text,
                                     2);

    JsonCpp::Value welcome;
    JsonCpp::CharReaderBuilder builder;
    std::string errors;
    const std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    TEST(first.messages.size() >= 1);
    TEST(reader->parse(first.messages[0].data(),
                       first.messages[0].data() + first.messages[0].size(),
                       &welcome,
                       &errors));
    const std::string token = welcome["token"].asString();
    TEST(!token.empty());

    std::this_thread::sleep_for(std::chrono::seconds(6));
    const auto reconnect = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        "{\"type\":\"hello\",\"token\":\"" + token + "\"}");

    TEST(reconnect.completed);
    TEST(reconnect.upgraded);
    TEST(reconnect.closed);
    TEST(!reconnect.messages.empty());
    TEST(reconnect.messages[0].find("\"type\":\"expired\"") !=
         std::string::npos);
    TEST(!fixture.serverFailed());
}

/** Refuses new UI sessions when the configured session limit is reached. */
TEST_CASE(TestWebServerEnforcesSessionLimit)
{
    auto &fixture = webServerFixture();
    bool limitReached = false;

    TEST(fixture.ready());
    for (int i = 0; i < 40 && !limitReached; ++i)
    {
        const auto result =
            fixture.sendWebSocketMessage("/ui",
                                         fixture.allowedOrigin(),
                                         R"({"type":"hello","token":""})",
                                         drogon::WebSocketMessageType::Text,
                                         2);
        TEST(result.completed);
        for (const auto &message : result.messages)
        {
            if (message.find("\"type\":\"unavailable\"") != std::string::npos)
                limitReached = true;
        }
    }

    TEST(limitReached);
    TEST(!fixture.serverFailed());
}
