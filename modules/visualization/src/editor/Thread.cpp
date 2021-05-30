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

/**
    @file Thread.cpp
*/

#include <Thread.hpp>

Thread::Thread(QObject *parent)
    : QThread(parent),
      state_(STATE_RUN),
      finished_(true),
      received_(false)
{
}

Thread::~Thread()
{
}

void Thread::init()
{
    QMutexLocker locker(&mutex_);

    if (!isRunning())
    {
        QThread::start(LowPriority);
    }
}

void Thread::start()
{
    setState(STATE_RUN);
}

void Thread::cancel()
{
    mutexCaller_.lock();
    setState(STATE_CANCEL);
    received_ = false;
    while (!received_)
    {
        conditionCaller_.wait(&mutexCaller_);
    }
    mutexCaller_.unlock();
}

void Thread::stop()
{
    setState(STATE_EXIT);
    wait();
}

void Thread::setState(State state)
{
    mutex_.lock();
    state_ = state;
    finished_ = false;
    condition_.wakeOne();
    mutex_.unlock();
}

void Thread::run()
{
    State state;
    bool finished = true;

    while (1)
    {
        mutex_.lock();
        while (state_ == STATE_RUN && finished_ && finished)
        {
            condition_.wait(&mutex_);
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
            conditionCaller_.wakeOne();
            mutexCaller_.unlock();
        }
        else
        {
            state = STATE_EXIT;
        }
        mutex_.unlock();

        if (state == STATE_EXIT)
        {
            return;
        }

        if (!finished)
        {
            finished = compute();
        }
    }
}
