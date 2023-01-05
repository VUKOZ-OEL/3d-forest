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

#include <Log.hpp>

std::shared_ptr<LogThread> globalLogThread;

LogThread::LogThread()
    : messageQueue_(100),
      messageQueueHead_(0),
      messageQueueTail_(0),
      running_(true),
      received_(false)
{
    thread_ = std::make_shared<std::thread>(&LogThread::run, this);
}

LogThread::~LogThread()
{
}

void LogThread::stop()
{
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);
    // sync start
    mutexlock.lock();
    running_ = false;
    received_ = false;
    mutexlock.unlock();
    // sync end
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

void LogThread::println(const std::string &msg)
{
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);
    // sync start
    mutexlock.lock();

    messageQueue_[messageQueueHead_] = msg;

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

    mutexlock.unlock();
    // sync end
    condition_.notify_one();
}

void LogThread::run()
{
    bool running;
    std::vector<std::string> messageQueue;
    size_t messageCount;
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);

    while (1)
    {
        // sync start
        mutexlock.lock();

        while (running_ && messageQueueHead_ == messageQueueTail_)
        {
            condition_.wait(mutexlock);
        }

        running = running_;

        if (!running)
        {
            mutexCaller_.lock();
            received_ = true;
            mutexCaller_.unlock();
            conditionCaller_.notify_one();
        }

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
            for (size_t i = messageQueueTail_; i < messageQueue_.size(); i++)
            {
                messageQueue[messageCount++] = messageQueue_[i];
            }

            for (size_t i = 0; i < messageQueueHead_; i++)
            {
                messageQueue[messageCount++] = messageQueue_[i];
            }
        }

        messageQueueTail_ = messageQueueHead_;

        mutexlock.unlock();
        // sync end

        try
        {
            for (size_t i = 0; i < messageCount; i++)
            {
                std::cout << messageQueue[i];
            }
            std::cout.flush();
        }
        catch (...)
        {
            // empty
        }

        if (!running)
        {
            return;
        }
    }
}
