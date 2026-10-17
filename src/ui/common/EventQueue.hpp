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

/** @file EventQueue.hpp */

#ifndef EVENT_QUEUE_HPP
#define EVENT_QUEUE_HPP

// Include std.
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** EventQueue. */
class EXPORT_UI_COMMON EventQueue
{
public:
    void post(std::function<void()> fn)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(fn));
        }
        m_cv.notify_one();
    }

    void runOne()
    {
        std::function<void()> fn;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&] { return !m_queue.empty(); });

            fn = std::move(m_queue.front());
            m_queue.pop();
        }

        fn();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::function<void()>> m_queue;
};

#include <WarningsEnable.hpp>

#endif /* EVENT_QUEUE_HPP */
