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

/** @file ThreadLoop.hpp */

#ifndef THREAD_LOOP_HPP
#define THREAD_LOOP_HPP

// Include std.
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <thread>

// Include 3D Forest.
class ThreadCallbackInterface;

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Thread Loop. */
class EXPORT_CORE ThreadLoop
{
public:
    ThreadLoop();
    virtual ~ThreadLoop();

    void setCallback(ThreadCallbackInterface *callback);

    void create();
    void start();
    void cancel();
    void stop();
    void wait();

    bool running();

    virtual bool next() = 0;

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

    // Thread state.
    std::mutex threadMutex_;
    std::condition_variable condition_;
    State state_;
    bool finished_;
    bool waiting_;

    // Synchronization to caller.
    std::mutex mutexCaller_;
    std::condition_variable conditionCaller_;
    bool received_;

    void setState(State state);
    void runLoop();

    friend std::ostream &operator<<(std::ostream &out,
                                    const ThreadLoop::State &in);
};

inline std::ostream &operator<<(std::ostream &out, const ThreadLoop::State &in)
{
    switch (in)
    {
        case ThreadLoop::STATE_RUN:
            out << "RUN";
            break;
        case ThreadLoop::STATE_CANCEL:
            out << "CANCEL";
            break;
        case ThreadLoop::STATE_EXIT:
            out << "EXIT";
            break;
        default:
            out << "UNKNOWN";
            break;
    }

    return out;
}

#include <WarningsEnable.hpp>

#endif /* THREAD_LOOP_HPP */
