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

/** @file WebSession.hpp */

#pragma once

// Include 3D Forest.
class WebApplication;

// Include 3rd party.
#include <drogon/WebSocketConnection.h>
#include <json/json.h>

// Include std.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebSession. */
class EXPORT_UI_WEB WebSession : public std::enable_shared_from_this<WebSession>
{
public:
    using Factory =
        std::function<std::unique_ptr<WebApplication>(WebSession &)>;
    using Task = std::function<void(WebApplication &)>;
    using Clock = std::chrono::steady_clock;

    WebSession(std::string token, Factory factory);
    ~WebSession();
    WebSession(const WebSession &) = delete;
    WebSession &operator=(const WebSession &) = delete;

    void start();
    void stop(); // Requests cancellation and joins; never call on session
                 // thread.
    bool post(Task task); // Bounded, thread-safe. False after stop/overflow.
    void wakeUp();        // Coalesced; thread-safe.
    bool isCurrentThread() const;
    bool expired(Clock::time_point now, std::chrono::seconds timeout) const;
    bool failed() const { return failed_.load(); }

    // Server transport entry points. These enqueue work on the session thread.
    bool attach(const drogon::WebSocketConnectionPtr &connection);
    void detach(const drogon::WebSocketConnectionPtr &connection);
    bool receive(const drogon::WebSocketConnectionPtr &connection,
                 JsonCpp::Value message);

private:
    friend class WebApplication;
    std::string token_;
    Factory factory_;
    std::thread worker_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<Task> tasks_;
    bool stopping_{false};
    bool wakePending_{false};
    bool connected_{false};
    drogon::WebSocketConnectionPtr ingressConnection_;
    Clock::time_point disconnectedAt_{Clock::now()};
    std::atomic_bool failed_{false};
    static constexpr std::size_t maxTasks_{1024};

    // Below: accessed only by worker_, including application destruction.
    std::unique_ptr<WebApplication> application_;
    drogon::WebSocketConnectionPtr connection_;
    JsonCpp::UInt64 revision_{0};
    JsonCpp::UInt64 awaitingAck_{0};
    Clock::time_point sentAt_{};

    void run() noexcept;
    void send(const JsonCpp::Value &message);
    void sendError(const std::string &message);
    bool canPublish() const;
    void publish(JsonCpp::Value message);
    void disconnect();
};

#include <WarningsEnable.hpp>
