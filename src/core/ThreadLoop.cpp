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

/** @file ThreadLoop.cpp */

// Include 3D Forest.
#include <ThreadCallbackInterface.hpp>
#include <ThreadLoop.hpp>

// Include local.
#define LOG_MODULE_NAME "ThreadLoop"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ThreadLoop::ThreadLoop()
    : callback_(nullptr),
      state_(STATE_RUN),
      finished_(true),
      waiting_(false),
      received_(false)
{
    LOG_DEBUG(<< "Create.");
}

ThreadLoop::~ThreadLoop()
{
    LOG_DEBUG(<< "Destroy.");
}

void ThreadLoop::setCallback(ThreadCallbackInterface *callback)
{
    callback_ = callback;
}

void ThreadLoop::create()
{
    LOG_DEBUG(<< "Create thread.");
    thread_ = std::make_shared<std::thread>(&ThreadLoop::runLoop, this);
}

void ThreadLoop::start()
{
    LOG_DEBUG(<< "Start thread.");
    setState(STATE_RUN);
}

void ThreadLoop::cancel()
{
    LOG_DEBUG(<< "Cancel thread.");
    std::unique_lock mutexlock(mutexCaller_);
    setState(STATE_CANCEL);
    received_ = false;
    while (!received_)
    {
        conditionCaller_.wait(mutexlock);
    }
    LOG_DEBUG(<< "Thread cancelled.");
}

bool ThreadLoop::running()
{
    State state;
    {
        std::unique_lock<std::mutex> mutexlock(mutex_);
        state = state_;
    }
    LOG_DEBUG(<< "Current state <" << state << ">.");
    return state == STATE_RUN;
}

void ThreadLoop::stop()
{
    LOG_DEBUG(<< "Stop thread.");
    setState(STATE_EXIT);
    thread_->join();
}

void ThreadLoop::wait()
{
    LOG_DEBUG(<< "Wait for thread.");
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);
    mutexlock.lock();
    waiting_ = true;
    condition_.notify_one();
    mutexlock.unlock();
    thread_->join();
}

void ThreadLoop::setState(State state)
{
    LOG_DEBUG(<< "Set state <" << state << ">.");
    std::unique_lock<std::mutex> mutexlock(mutex_);
    state_ = state;
    LOG_DEBUG(<< "State <" << state << "> is set.");
    finished_ = false;
    condition_.notify_one();
}

void ThreadLoop::runLoop()
{
    LOG_DEBUG(<< "Run.");

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
                mutexlock.lock();
                state_ = STATE_CANCEL;
                mutexlock.unlock();
            }
            catch (...)
            {
                LOG_ERROR(<< "Unknown error.");
                mutexlock.lock();
                state_ = STATE_CANCEL;
                mutexlock.unlock();
            }
        }

        if (waiting && finished)
        {
            LOG_DEBUG(<< "Terminate this thread.");
            return;
        }
    }
}
