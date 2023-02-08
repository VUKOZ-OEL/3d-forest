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

/** @file Thread.cpp */

#include <Thread.hpp>
#include <ThreadCallbackInterface.hpp>

#define LOG_MODULE_NAME "Thread"
#include <Log.hpp>

Thread::Thread()
    : callback_(nullptr),
      state_(STATE_RUN),
      finished_(true),
      waiting_(false),
      received_(false)
{
    LOG_DEBUG(<< "Called.");
}

Thread::~Thread()
{
    LOG_DEBUG(<< "Called.");
}

void Thread::setCallback(ThreadCallbackInterface *callback)
{
    callback_ = callback;
}

void Thread::create()
{
    LOG_DEBUG(<< "Called.");
    thread_ = std::make_shared<std::thread>(&Thread::runLoop, this);
}

void Thread::start()
{
    LOG_DEBUG(<< "Called.");
    setState(STATE_RUN);
}

void Thread::cancel()
{
    LOG_DEBUG(<< "Called.");
    std::unique_lock mutexlock(mutexCaller_);
    setState(STATE_CANCEL);
    received_ = false;
    while (!received_)
    {
        conditionCaller_.wait(mutexlock);
    }
}

void Thread::stop()
{
    LOG_DEBUG(<< "Called.");
    setState(STATE_EXIT);
    thread_->join();
}

void Thread::wait()
{
    LOG_DEBUG(<< "Called.");
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);
    mutexlock.lock();
    waiting_ = true;
    condition_.notify_one();
    mutexlock.unlock();
    thread_->join();
}

void Thread::setState(State state)
{
    LOG_DEBUG(<< "Called with parameter state <" << state << ">.");
    std::unique_lock<std::mutex> mutexlock(mutex_);
    state_ = state;
    finished_ = false;
    condition_.notify_one();
}

void Thread::runLoop()
{
    LOG_DEBUG(<< "Called.");

    State state;
    bool finished = true;
    bool waiting = false;
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);

    while (1)
    {
        mutexlock.lock();
        while (state_ == STATE_RUN && finished_ && finished && !waiting_)
        {
            LOG_DEBUG(<< "Thread is waiting for a new signal.");
            condition_.wait(mutexlock);
        }

        if (state_ == STATE_RUN)
        {
            LOG_DEBUG(<< "New signal received. Thread state is STATE_RUN.");
            state = STATE_RUN;

            if (!finished_)
            {
                finished = false;
                finished_ = true;
            }
        }
        else if (state_ == STATE_CANCEL)
        {
            LOG_DEBUG(<< "New signal received. Thread state is STATE_CANCEL.");
            state_ = STATE_RUN;
            state = STATE_RUN;
            finished_ = true;
            finished = true;

            mutexCaller_.lock();
            received_ = true;
            conditionCaller_.notify_one();
            mutexCaller_.unlock();
        }
        else
        {
            LOG_DEBUG(<< "New signal received. Thread state is STATE_EXIT.");
            state = STATE_EXIT;
        }

        waiting = waiting_;
        LOG_DEBUG(<< "Thread is waiting <" << waiting << ">.");
        mutexlock.unlock();

        if (state == STATE_EXIT)
        {
            LOG_DEBUG(<< "Stop this thread.");
            return;
        }

        if (!finished)
        {
            LOG_DEBUG(<< "Call next function.");
            try
            {
                finished = !next();
            }
            catch (std::exception &e)
            {
                LOG_ERROR(<< "Error message <" << e.what() << ">.");
            }
            catch (...)
            {
                LOG_ERROR(<< "Unknown error.");
            }
        }

        if (waiting && finished)
        {
            LOG_DEBUG(<< "Terminate this thread.");
            return;
        }
    }
}
