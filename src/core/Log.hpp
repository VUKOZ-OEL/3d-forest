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

#include <condition_variable>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <ExportCore.hpp>

/** Log Type. */
enum EXPORT_CORE LogType
{
    LOG_TYPE_DEBUG,
    LOG_TYPE_WARNING,
    LOG_TYPE_ERROR,
    LOG_TYPE_INFO,
    LOG_TYPE_PRINT
};

/** Log Message. */
class EXPORT_CORE LogMessage
{
public:
    LogType type;
    size_t threadId;
    std::string time;
    std::string module;
    std::string function;
    std::string text;

    void set(LogType type_,
             size_t threadId_,
             const std::string &time_,
             const std::string &module_,
             const std::string &function_,
             const std::string &text_)
    {
        type = type_;
        threadId = threadId_;
        time = time_;
        module = module_;
        function = function_;
        text = text_;
    }

    const char *typeString() const;
    static const char *typeString(int type_);
};

/** Log Thread Callback Interface. */
class EXPORT_CORE LogThreadCallbackInterface
{
public:
    virtual ~LogThreadCallbackInterface() = default;
    virtual void println(const LogMessage &message) = 0;
    virtual void flush() = 0;
};

/** Logger For Standard Output. */
class EXPORT_CORE LoggerStdout : public LogThreadCallbackInterface
{
public:
    LoggerStdout() = default;
    virtual ~LoggerStdout() = default;
    virtual void println(const LogMessage &message);
    virtual void flush();
};

/** Log Thread. */
class EXPORT_CORE LogThread
{
public:
    LogThread();
    ~LogThread();

    void setCallback(LogThreadCallbackInterface *callback);

    void println(LogType type,
                 const char *module,
                 const char *function,
                 const std::string &text);

    void stop();

private:
    std::vector<LogMessage> messageQueue_;
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

extern std::shared_ptr<LogThread> EXPORT_CORE globalLogThread;
extern std::shared_ptr<LoggerStdout> EXPORT_CORE globalLoggerStdout;

#define LOG_MESSAGE(type, module, msg)                                         \
    do                                                                         \
    {                                                                          \
        std::stringstream stream;                                              \
        stream << "" msg;                                                      \
        if (globalLogThread)                                                   \
        {                                                                      \
            globalLogThread->println(type, module, __func__, stream.str());    \
        }                                                                      \
    } while (false)

#if defined(LOG_MODULE_DEBUG_ENABLED)
    #define LOG_DEBUG(msg) LOG_MESSAGE(LOG_TYPE_DEBUG, LOG_MODULE_NAME, msg)
#else
    #define LOG_DEBUG(msg)
#endif /* LOG_MODULE_DEBUG_ENABLED */

#define LOG_TRACE_UNKNOWN(msg)

#if 0
    #define LOG_TRACE_UPDATE_VIEW(msg)                                         \
        LOG_MESSAGE(LOG_TYPE_DEBUG, LOG_MODULE_NAME, msg);
#else
    #define LOG_TRACE_UPDATE_VIEW(msg)
#endif

#define LOG_WARNING(msg) LOG_MESSAGE(LOG_TYPE_WARNING, LOG_MODULE_NAME, msg)
#define LOG_ERROR(msg) LOG_MESSAGE(LOG_TYPE_ERROR, LOG_MODULE_NAME, msg)
#define LOG_INFO(msg) LOG_MESSAGE(LOG_TYPE_INFO, LOG_MODULE_NAME, msg)
#define LOG_PRINT(msg) LOG_MESSAGE(LOG_TYPE_PRINT, "", msg)

#define LOGGER_START_STDOUT                                                    \
    do                                                                         \
    {                                                                          \
        globalLogThread = std::make_shared<LogThread>();                       \
        globalLoggerStdout = std::make_shared<LoggerStdout>();                 \
        globalLogThread->setCallback(globalLoggerStdout.get());                \
    } while (false)

#define LOGGER_STOP_STDOUT                                                     \
    do                                                                         \
    {                                                                          \
        globalLogThread->stop();                                               \
        globalLogThread->setCallback(nullptr);                                 \
    } while (false)

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
