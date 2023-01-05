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

Thread::Thread()
    : callback_(nullptr),
      state_(STATE_RUN),
      finished_(true),
      waiting_(false),
      received_(false)
{
}

Thread::~Thread()
{
}

void Thread::setCallback(ThreadCallbackInterface *callback)
{
    callback_ = callback;
}

void Thread::create()
{
    thread_ = std::make_shared<std::thread>(&Thread::runLoop, this);
}

void Thread::start()
{
    setState(STATE_RUN);
}

void Thread::cancel()
{
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
    setState(STATE_EXIT);
    thread_->join();
}

void Thread::wait()
{
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);
    mutexlock.lock();
    waiting_ = true;
    condition_.notify_one();
    mutexlock.unlock();
    thread_->join();
}

void Thread::setState(State state)
{
    std::unique_lock<std::mutex> mutexlock(mutex_);
    state_ = state;
    finished_ = false;
    condition_.notify_one();
}

void Thread::runLoop()
{
    State state;
    bool finished = true;
    bool waiting = false;
    std::unique_lock<std::mutex> mutexlock(mutex_, std::defer_lock);

    while (1)
    {
        mutexlock.lock();
        while (state_ == STATE_RUN && finished_ && finished && !waiting_)
        {
            condition_.wait(mutexlock);
        }

        if (state_ == STATE_RUN)
        {
            state = STATE_RUN;

            if (!finished_)
            {
                finished = false;
                finished_ = true;
            }
        }
        else if (state_ == STATE_CANCEL)
        {
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
            state = STATE_EXIT;
        }

        waiting = waiting_;
        mutexlock.unlock();

        if (state == STATE_EXIT)
        {
            return;
        }

        if (!finished)
        {
            try
            {
                finished = compute();
            }
            catch (std::exception &e)
            {
                // LOG_MODULE("error: " << e.what());
            }
            catch (...)
            {
                // LOG_MODULE("error: unknown");
            }
        }

        if (waiting && finished)
        {
            return;
        }
    }
}
