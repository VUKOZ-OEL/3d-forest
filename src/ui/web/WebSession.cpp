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

/** @file WebSession.cpp */

// Include 3D Forest.
#include <WebApplication.hpp>
#include <WebSession.hpp>

// Include std.
#include <iostream>
#include <stdexcept>
#include <utility>

// Include local.
#define LOG_MODULE_NAME "WebSession"
#include <Log.hpp>

namespace
{
thread_local const WebSession *currentSession = nullptr;
}

WebSession::WebSession(std::string token, Factory factory)
    : token_(std::move(token)),
      factory_(std::move(factory))
{
    if (!factory_)
        throw std::invalid_argument("Missing application factory");
}
WebSession::~WebSession()
{
    stop();
}

void WebSession::start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (worker_.joinable() || stopping_)
        throw std::logic_error("Session already started/stopped");
    worker_ = std::thread([this] { run(); });
}
void WebSession::stop()
{
    if (isCurrentThread())
        std::terminate(); // Owner must join from another thread.
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    condition_.notify_one();
    if (worker_.joinable())
        worker_.join();
}
bool WebSession::isCurrentThread() const
{
    return currentSession == this;
}

bool WebSession::post(Task task)
{
    if (!task)
        return false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || failed_ || tasks_.size() >= maxTasks_)
            return false;
        tasks_.push_back(std::move(task));
    }
    condition_.notify_one();
    return true;
}
void WebSession::wakeUp()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || failed_)
            return;
        wakePending_ = true;
    }
    condition_.notify_one();
}
bool WebSession::expired(Clock::time_point now,
                         std::chrono::seconds timeout) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !connected_ && now - disconnectedAt_ >= timeout;
}

bool WebSession::attach(const drogon::WebSocketConnectionPtr &connection)
{
    // Mark immediately: the reaper must not expire a queued reattachment.
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || failed_ || tasks_.size() >= maxTasks_)
            return false;
        connected_ = true;
        ingressConnection_ = connection;
        tasks_.push_back(
            [this, connection](WebApplication &app)
            {
                if (connection_ && connection_ != connection)
                {
                    connection_->send(
                        R"({"type":"replaced","message":"Session opened in another tab"})");
                    connection_->shutdown();
                }
                connection_ = connection;
                awaitingAck_ = 0;
                JsonCpp::Value welcome{JsonCpp::objectValue};
                welcome["type"] = "welcome";
                welcome["token"] = token_;
                send(welcome);
                app.forceSnapshot();
            });
    }
    condition_.notify_one();
    return true;
}

void WebSession::detach(const drogon::WebSocketConnectionPtr &connection)
{
    // Lifecycle work must not be dropped when the regular event queue is full.
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || failed_)
            return;
        tasks_.push_back(
            [this, connection](WebApplication &)
            {
                // An old socket closing must not detach its replacement.
                if (connection_ == connection)
                    disconnect();
            });
    }
    condition_.notify_one();
}

bool WebSession::receive(const drogon::WebSocketConnectionPtr &connection,
                         JsonCpp::Value message)
{
    return post(
        [this, connection, message = std::move(message)](WebApplication &app)
        {
            if (connection_ != connection)
                return;
            const auto type = message["type"].asString();
            if (type == "ack")
            {
                if (!message["revision"].isUInt64() || awaitingAck_ == 0 ||
                    message["revision"].asUInt64() != awaitingAck_)
                    throw std::invalid_argument(
                        "Invalid state acknowledgement");
                awaitingAck_ = 0;
            }
            else if (type == "event")
                app.dispatch(message);
            else if (type == "resync")
                app.forceSnapshot();
            else
                throw std::invalid_argument("Unknown message type");
        });
}

void WebSession::send(const JsonCpp::Value &message)
{
    if (!connection_ || !connection_->connected())
        return;
    JsonCpp::StreamWriterBuilder writer;
    writer["indentation"] = "";
    connection_->send(JsonCpp::writeString(writer, message));
}
void WebSession::sendError(const std::string &message)
{
    JsonCpp::Value error{JsonCpp::objectValue};
    error["type"] = "error";
    error["message"] = message;
    send(error);
}
bool WebSession::canPublish() const
{
    return connection_ && connection_->connected() && awaitingAck_ == 0;
}
void WebSession::publish(JsonCpp::Value message)
{
    message["revision"] = ++revision_;
    awaitingAck_ = revision_;
    sentAt_ = Clock::now();
    send(message);
}
void WebSession::disconnect()
{
    auto old = std::move(connection_);
    if (old)
        old->forceClose();
    awaitingAck_ = 0;
    std::lock_guard<std::mutex> lock(mutex_);
    if (ingressConnection_ == old)
    {
        ingressConnection_.reset();
        connected_ = false;
        disconnectedAt_ = Clock::now();
    }
}

void WebSession::run() noexcept
{
    currentSession = this;
    try
    {
        application_ = factory_(*this);
        factory_ = {};
        if (!application_)
            throw std::runtime_error("Factory returned a null application");
        for (;;)
        {
            Task task;
            bool wake = false;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait_for(lock,
                                    std::chrono::seconds(1),
                                    [this] {
                                        return stopping_ || wakePending_ ||
                                               !tasks_.empty();
                                    });
                if (stopping_)
                    break;
                wake = wakePending_;
                wakePending_ = false;
                if (!tasks_.empty())
                {
                    task = std::move(tasks_.front());
                    tasks_.pop_front();
                }
            }
            try
            {
                if (task)
                    task(*application_);
                if (wake)
                    application_->handleWakeUp();
                if (connection_ &&
                    (!connection_->connected() ||
                     (awaitingAck_ &&
                      Clock::now() - sentAt_ > std::chrono::seconds(30))))
                    disconnect();
                application_->flush();
            }
            catch (const std::invalid_argument &e)
            {
                // Protocol/adaptor rejection: retain the session but correct UI
                // state.
                sendError(e.what());
                application_->forceSnapshot();
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Web session failed: " << e.what() << '\n';
        failed_ = true;
    }
    catch (...)
    {
        std::cerr << "Web session failed: unknown exception\n";
        failed_ = true;
    }
    disconnect();
    drogon::WebSocketConnectionPtr pendingConnection;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
        tasks_.clear();
        pendingConnection = std::move(ingressConnection_);
        connected_ = false;
    }
    if (pendingConnection)
        pendingConnection->forceClose();
    // Plugins/widgets are destroyed on the same thread that created them.
    application_.reset();
    currentSession = nullptr;
}
