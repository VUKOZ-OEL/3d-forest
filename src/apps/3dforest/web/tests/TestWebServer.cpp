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
#include <trantor/net/EventLoopThread.h>
#include <trantor/net/TcpServer.h>
#include <trantor/utils/MsgBuffer.h>

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
#include <TextEdit.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <Test.hpp>
#include <WebApplication.hpp>
#include <WebServer.hpp>

// Include std.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
class WebServerProtocolTestPlugin final : public Plugin
{
public:
    explicit WebServerProtocolTestPlugin(int localServicePort)
        : localServicePort_(localServicePort)
    {
    }

    const char *name() const override { return "WebServerProtocolTest"; }

    void initialize(Application *application) override
    {
        auto *webApplication = dynamic_cast<WebApplication *>(application);
        if (!webApplication)
            throw std::invalid_argument("Expected a WebApplication");

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
    template <typename T>
    void add(const std::string &name)
    {
        auto widget = std::make_unique<T>();
        widget->setName(name);
        root_->addWidget(widget.release());
    }

    std::unique_ptr<Splitter> root_;
    int localServicePort_;
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
        localService_ = std::make_unique<trantor::TcpServer>(
            localServiceLoop_->getLoop(),
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
                {
                    std::lock_guard<std::mutex> lock(upstreamMutex_);
                    lastUpstreamRequest_ = request;
                }
                static const std::string response =
                    "HTTP/1.1 201 Created\r\n"
                    "Content-Length: 12\r\n"
                    "Content-Type: text/plain\r\n"
                    "X-Upstream: preserved\r\n"
                    "Connection: close\r\n"
                    "Keep-Alive: timeout=5\r\n"
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
            [localServicePort = localServicePort_](WebSession &session)
            {
                auto application = std::make_unique<WebApplication>(session);
                auto plugin =
                    std::make_shared<WebServerProtocolTestPlugin>(
                        localServicePort);
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

    Response sendHttpRequest(const std::string &path,
                             drogon::HttpMethod method,
                             const std::string &body,
                             const std::vector<std::pair<std::string,
                                                         std::string>> &headers = {})
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

    WebSocketResult sendWebSocketMessage(const std::string &path,
                                         const std::string &origin,
                                         const std::string &message,
                                         drogon::WebSocketMessageType type =
                                             drogon::WebSocketMessageType::Text,
                                         std::size_t expectedMessages = 0,
                                         std::function<std::vector<std::string>(
                                             const JsonCpp::Value &)> afterSnapshot = {})
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
            [result](const drogon::WebSocketClientPtr &)
            {
                {
                    std::lock_guard<std::mutex> lock(result->mutex);
                    result->closed = true;
                }
                result->condition.notify_one();
            });

        auto request = drogon::HttpRequest::newHttpRequest();
        request->setPath(path);
        request->addHeader("origin", origin);
        std::weak_ptr<drogon::WebSocketClient> weakWebSocket = webSocket;
        webSocket->connectToServer(
            request,
            [result, weakWebSocket, message, type](
                drogon::ReqResult requestResult,
                const drogon::HttpResponsePtr &response,
                const drogon::WebSocketClientPtr &)
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
                if (upgraded)
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
                std::chrono::seconds(3),
                [&] {
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
    std::unique_ptr<trantor::EventLoopThread> clientLoop_;
    std::unique_ptr<trantor::EventLoopThread> localServiceLoop_;
    std::unique_ptr<trantor::TcpServer> localService_;
    int localServicePort_{0};
    mutable std::mutex upstreamMutex_;
    std::string lastUpstreamRequest_;
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
        "{\"token\":\"\"}",
        "{\"type\":3}",
        "{\"type\":\"hello\",\"type\":\"hello\",\"token\":\"\"}",
        "{\"type\":\"hello\",\"token\":\"\"} trailing",
        deepJson,
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

TEST_CASE(TestWebServerRejectsBinaryUiMessages)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
        fixture.allowedOrigin(),
        "{}",
        drogon::WebSocketMessageType::Binary);

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(result.upgraded);
    TEST(result.closed);
    TEST(!fixture.serverFailed());
}

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

TEST_CASE(TestWebServerPublishesTestPluginWidgetsWithoutNavigation)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto parse = [&builder, &errors](const std::string &text,
                                           JsonCpp::Value &value)
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

    const std::vector<std::string> expectedTypes = {
        "CheckBox",     "ColorSwitch", "ComboBox",   "DoubleRangeSlider",
        "DoubleSpinBox", "GroupBox",   "Label",      "LineEdit",
        "ProgressBar",  "PushButton",  "RadioButton", "Slider",
        "SpinBox",      "Splitter",    "TableWidget", "TextEdit",
        "ToolBar",      "ToolButton",  "TreeWidget"};
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

TEST_CASE(TestWebServerReconnectsWithSessionToken)
{
    auto &fixture = webServerFixture();
    const auto first = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto parse = [&builder, &errors](const std::string &text,
                                           JsonCpp::Value &value)
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
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto result = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto parse = [&builder, &errors](const std::string &text,
                                           JsonCpp::Value &value)
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

TEST_CASE(TestWebServerRejectsUiWebSocketWithoutOrigin)
{
    auto &fixture = webServerFixture();
    const auto result = fixture.sendWebSocketMessage("/ui", "", "{}");

    TEST(fixture.ready());
    TEST(result.completed);
    TEST(!result.upgraded || result.closed);
    TEST(!fixture.serverFailed());
}

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

TEST_CASE(TestWebServerProxiesMarteloscopeHttpRequest)
{
    auto &fixture = webServerFixture();
    const auto hello = fixture.sendWebSocketMessage(
        "/ui",
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
    const auto response = fixture.sendHttpRequest(
        proxyPath,
        drogon::Post,
        "request-body",
        {{"X-Test-Request", "passed"},
         {"Connection", "keep-alive"},
         {"Keep-Alive", "timeout=5"},
         {"TE", "trailers"},
         {"Trailer", "X-Checksum"},
         {"Proxy-Authorization", "secret"}});

    TEST(response.first == drogon::ReqResult::Ok);
    TEST(response.second);
    if (response.second)
    {
        TEST(response.second->statusCode() == drogon::k201Created);
        TEST(response.second->body() == "upstream-ok!");
        TEST(response.second->getHeader("x-upstream") == "preserved");
        TEST(response.second->getHeader("referrer-policy") == "no-referrer");
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

    const auto getResponse = fixture.sendHttpRequest(
        "/marteloscope/" + token + "/health",
        drogon::Get,
        "");
    TEST(getResponse.first == drogon::ReqResult::Ok);
    TEST(getResponse.second &&
         getResponse.second->statusCode() == drogon::k201Created);
    TEST(!fixture.serverFailed());
}

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

TEST_CASE(TestWebServerRejectsMissingDocumentRoot)
{
    WebServer::Options options;
    options.documentRoot =
        (std::filesystem::temp_directory_path() /
         ("3DForestMissingWebRoot" +
          std::to_string(std::chrono::steady_clock::now()
                             .time_since_epoch()
                             .count())))
            .string();
    WebServer server(options);
    server.setApplicationFactory(
        [](WebSession &) { return std::unique_ptr<WebApplication>{}; });

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
