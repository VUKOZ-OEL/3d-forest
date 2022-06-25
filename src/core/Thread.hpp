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

/** @file Thread.hpp */

#ifndef THREAD_HPP
#define THREAD_HPP

#include <condition_variable>
#include <mutex>
#include <thread>

#include <ExportCore.hpp>

class ThreadCallbackInterface;

/** Thread. */
class EXPORT_CORE Thread
{
public:
    Thread();
    virtual ~Thread();

    void setCallback(ThreadCallbackInterface *callback);

    void create();
    void start();
    void cancel();
    void stop();
    void wait();

    virtual bool compute() = 0;

protected:
    std::shared_ptr<std::thread> thread_;
    ThreadCallbackInterface *callback_;

    /** Thread state. */
    enum State
    {
        STATE_RUN,
        STATE_CANCEL,
        STATE_EXIT
    };

    // Thread state
    std::mutex mutex_;
    std::condition_variable condition_;
    State state_;
    bool finished_;
    bool waiting_;

    // Synchronization to caller
    std::mutex mutexCaller_;
    std::condition_variable conditionCaller_;
    bool received_;

    void setState(State state);
    void runLoop();
};

#endif /* THREAD_HPP */
