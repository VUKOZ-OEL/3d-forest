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

/** @file Log.cpp */

// Include 3D Forest.
#include <Time.hpp>

// Include local.
#define LOG_MODULE_NAME "Log"
#include <Log.hpp>

std::shared_ptr<LogThread> globalLogThread;
std::shared_ptr<LoggerStdout> globalLoggerStdout;
std::shared_ptr<LoggerFile> globalLoggerFile;

LogThread::LogThread()
    : messageQueue_(10000U),
      messageQueueHead_(0),
      messageQueueTail_(0),
      callback_(nullptr),
      running_(true),
      received_(false)
{
    thread_ = std::make_shared<std::thread>(&LogThread::run, this);
}

LogThread::~LogThread()
{
}

void LogThread::setCallback(LogThreadCallbackInterface *callback)
{
    {
        std::unique_lock<std::mutex> mutexlock(logMutex_);
        callback_ = callback;
    }
    condition_.notify_one();
}

void LogThread::stop()
{
    {
        std::unique_lock<std::mutex> mutexlock(logMutex_);
        running_ = false;
        received_ = false;
    }
    condition_.notify_one();

    {
        std::unique_lock mutexlockCaller(mutexCaller_);
        while (!received_)
        {
            conditionCaller_.wait(mutexlockCaller);
        }
    }

    thread_->join();
}

void LogThread::println(LogType type,
                        const char *module,
                        const char *function,
                        const std::string &text)
{
    std::string timeString = Time::strftime();
    size_t threadId = std::hash<std::thread::id>()(std::this_thread::get_id());

    {
        std::unique_lock<std::mutex> mutexlock(logMutex_);

        messageQueue_[messageQueueHead_]
            .set(type, threadId, timeString, module, function, text);

        if (messageQueueHead_ + 1 == messageQueue_.size())
        {
            messageQueueHead_ = 0;
        }
        else
        {
            messageQueueHead_++;
        }

        if (messageQueueHead_ == messageQueueTail_)
        {
            if (messageQueueTail_ + 1 == messageQueue_.size())
            {
                messageQueueTail_ = 0;
            }
            else
            {
                messageQueueTail_++;
            }
        }
    }
    condition_.notify_one();
}

void LogThread::run()
{
    bool running;
    LogThreadCallbackInterface *callback = nullptr;
    std::vector<LogMessage> messageQueue;
    size_t messageCount;
    std::unique_lock<std::mutex> mutexlock(logMutex_, std::defer_lock);

    while (1)
    {
        // Sync start.
        mutexlock.lock();

        while (running_ && messageQueueHead_ == messageQueueTail_ &&
               callback == callback_)
        {
            condition_.wait(mutexlock);
        }

        running = running_;
        callback = callback_;

        if (!running)
        {
            mutexCaller_.lock();
            received_ = true;
            mutexCaller_.unlock();
            conditionCaller_.notify_one();
        }

        if (callback)
        {
            if (messageQueue.size() != messageQueue_.size())
            {
                messageQueue.resize(messageQueue_.size());
            }

            messageCount = 0;

            if (messageQueueTail_ < messageQueueHead_)
            {
                for (size_t i = messageQueueTail_; i < messageQueueHead_; i++)
                {
                    messageQueue[messageCount++] = messageQueue_[i];
                }
            }
            else if (messageQueueHead_ < messageQueueTail_)
            {
                for (size_t i = messageQueueTail_; i < messageQueue_.size();
                     i++)
                {
                    messageQueue[messageCount++] = messageQueue_[i];
                }

                for (size_t i = 0; i < messageQueueHead_; i++)
                {
                    messageQueue[messageCount++] = messageQueue_[i];
                }
            }

            messageQueueTail_ = messageQueueHead_;
        }

        mutexlock.unlock();
        // Sync end.

        try
        {
            if (callback)
            {
                for (size_t i = 0; i < messageCount; i++)
                {
                    callback->println(messageQueue[i]);
                }
                callback->flush();
            }
        }
        catch (...)
        {
            // Empty.
        }

        if (!running)
        {
            return;
        }
    }
}

const char *LogMessage::typeString() const
{
    return typeString(type);
}

const char *LogMessage::typeString(int type_)
{
    switch (type_)
    {
        case LOG_TYPE_DEBUG:
            return " DBG ";
        case LOG_TYPE_WARNING:
            return " WRN ";
        case LOG_TYPE_ERROR:
            return " ERR ";
        case LOG_TYPE_INFO:
            return " INF ";
        default:
            break;
    }

    return " N/A ";
}

void LoggerStdout::println(const LogMessage &message)
{
    if (message.type == LOG_TYPE_PRINT)
    {
        std::cout << message.text << std::endl;
    }
    else
    {
        std::cout << message.time << message.typeString() << message.text
                  << " [" << message.module << ":" << message.function << "]"
                  << std::endl;
    }
}

void LoggerStdout::flush()
{
    // Empty.
}

LoggerFile::LoggerFile(const std::string &fileName)
{
    file_.open(fileName, "w+t");
}

void LoggerFile::println(const LogMessage &message)
{
    std::string line =
        message.time + std::string(LogMessage::typeString(message.type)) +
        message.text + " [" + message.module + ":" + message.function + "] " +
        std::to_string(message.threadId);

    file_.write(line + "\n");
}

void LoggerFile::flush()
{
    // Empty.
}
