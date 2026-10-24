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
#include <trantor/net/EventLoopThread.h>

// Include 3D Forest.
#include <Test.hpp>
#include <WebApplication.hpp>
#include <WebServer.hpp>

// Include std.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace
{
class WebServerTestFixture
{
public:
    using Response = std::pair<drogon::ReqResult, drogon::HttpResponsePtr>;

    WebServerTestFixture()
        : documentRoot_(makeDocumentRoot()),
          options_(makeOptions(documentRoot_)),
          server_(options_)
    {
        std::filesystem::create_directories(documentRoot_);
        std::ofstream indexFile(documentRoot_ / "index.html");
        indexFile << "web server test page";
        indexFile.close();

        server_.setApplicationFactory(
            [](WebSession &)
            { return std::unique_ptr<WebApplication>{}; });
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

    bool ready() const { return ready_; }
    bool serverFailed() const { return serverFailed_; }

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
            20000 + (std::chrono::steady_clock::now()
                         .time_since_epoch()
                         .count() % 40000));
        options.documentRoot = root.string();
        options.allowedOrigin = "http://127.0.0.1";
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
        client_.reset();
        clientLoop_.reset();
        std::error_code error;
        std::filesystem::remove_all(documentRoot_, error);
        stopped_ = true;
    }

    std::filesystem::path documentRoot_;
    WebServer::Options options_;
    WebServer server_;
    std::thread serverThread_;
    std::atomic_bool serverFinished_{false};
    std::atomic_bool serverFailed_{false};
    std::unique_ptr<trantor::EventLoopThread> clientLoop_;
    drogon::HttpClientPtr client_;
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
