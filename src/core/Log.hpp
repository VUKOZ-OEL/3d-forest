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

/** @file Log.hpp */

#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <ExportCore.hpp>

/** Log Thread Callback Interface. */
class EXPORT_CORE LogThreadCallbackInterface
{
public:
    virtual ~LogThreadCallbackInterface() = default;
    virtual void println(const std::string &msg) = 0;
    virtual void flush() = 0;
};

/** Log Thread. */
class EXPORT_CORE LogThread
{
public:
    LogThread();
    virtual ~LogThread();

    void println(const std::string &msg);
    void setCallback(LogThreadCallbackInterface *callback);
    void stop();

private:
    std::vector<std::string> messageQueue_;
    size_t messageQueueHead_;
    size_t messageQueueTail_;

    std::shared_ptr<std::thread> thread_;
    std::mutex mutex_;
    std::condition_variable condition_;

    std::mutex mutexCaller_;
    std::condition_variable conditionCaller_;

    LogThreadCallbackInterface *callback_;

    bool running_;
    bool received_;

    void run();
};

extern std::shared_ptr<LogThread> globalLogThread;

#define LOG_ENABLE 1

//#define LOG_MODULE(module, msg)
#define LOG_MODULE(module, msg)                                                \
    do                                                                         \
    {                                                                          \
        std::stringstream str;                                                 \
        str << (module) << "::" << __func__ << ": " msg;                       \
        if (globalLogThread)                                                   \
        {                                                                      \
            globalLogThread->println(str.str());                               \
        }                                                                      \
    } while (false)

#define LOG_UPDATE_VIEW(module, msg)
//#define LOG_UPDATE_VIEW(module, msg) LOG_MODULE(module, msg)
#define LOG_FILTER(module, msg)
//#define LOG_FILTER(module, msg) LOG_MODULE(module, msg)

template <typename T>
inline std::ostream &operator<<(std::ostream &os, std::unordered_set<T> vec)
{
    os << "{ ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
    os << "}";
    return os;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, std::set<T> vec)
{
    os << "{ ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
    os << "}";
    return os;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, std::vector<T> vec)
{
    os << "{ ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
    os << "}";
    return os;
}

#endif /* LOG_HPP */
