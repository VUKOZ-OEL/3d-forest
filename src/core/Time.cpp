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

/** @file Time.cpp */

#include <cstdio>

#define _POSIX_THREAD_SAFE_FUNCTIONS

#if defined(_MSC_VER)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif /* _MSC_VER */

#include <ctime>

#include <Time.hpp>

#define LOG_MODULE_NAME "Time"
#include <Log.hpp>

double Time::realTime()
{
#if defined(_MSC_VER)
    FILETIME tp;
    GetSystemTimeAsFileTime(&tp);
    uint64_t t =
        (static_cast<uint64_t>(tp.dwHighDateTime) << 32) + tp.dwLowDateTime;
    return (static_cast<double>(t) * 1e-7) - 11644473600.; // sec 1 Jan 1970
#else
    struct timeval tp;

    (void)::gettimeofday(&tp, NULL);

    return static_cast<double>(tp.tv_sec) +
           (1e-6 * static_cast<double>(tp.tv_usec));
#endif /* _MSC_VER */
}

uint64_t Time::realTime64()
{
#if defined(_MSC_VER)
    LARGE_INTEGER fq, t;
    QueryPerformanceFrequency(&fq);
    QueryPerformanceCounter(&t);
    return (1000000UL * t.QuadPart) / fq.QuadPart;
#else
    struct timeval tp;

    (void)::gettimeofday(&tp, NULL);

    return (static_cast<uint64_t>(tp.tv_sec) << 32) +
           static_cast<uint64_t>(tp.tv_usec);
#endif /* _MSC_VER */
}

void Time::msleep(long milliseconds)
{
#if defined(_MSC_VER)
    Sleep(milliseconds);
#else
    struct timespec request;

    request.tv_sec = milliseconds / 1000L;
    request.tv_nsec = (milliseconds % 1000L) * 1000000L;

    (void)::nanosleep(&request, &request);
#endif /* _MSC_VER */
}

std::string Time::strftime(const char *format)
{
#if defined(_MSC_VER)
    // Get the current system date and time in UTC format.
    FILETIME utp;
    GetSystemTimeAsFileTime(&utp);

    // Convert UTC-based time into a local time.
    FILETIME ltp;
    FileTimeToLocalFileTime(&utp, &ltp);

    // Convert Epoch time into a month, day, year, weekday, hour, etc.
    SYSTEMTIME stp;
    FileTimeToSystemTime(&ltp, &stp);

    // Format date and time structure to a character string.
    char buffer[128];
    (void)std::snprintf(buffer,
                        sizeof(buffer),
                        "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                        static_cast<int>(stp.wYear),
                        static_cast<int>(stp.wMonth),
                        static_cast<int>(stp.wDay),
                        static_cast<int>(stp.wHour),
                        static_cast<int>(stp.wMinute),
                        static_cast<int>(stp.wSecond),
                        static_cast<int>(stp.wMilliseconds));

    // Return formatted time string.
    return std::string(buffer);
#else
    // Get the number of seconds and microseconds since the Epoch.
    struct timeval tp;
    (void)::gettimeofday(&tp, nullptr);

    // Convert the number of seconds since the Epoch into local calendar time.
    std::time_t seconds = static_cast<std::time_t>(tp.tv_sec);
    struct std::tm calendarTime;
    (void)localtime_r(&seconds, &calendarTime);

    // Convert calendar time to a character string like '2020-06-05 08:20:35'.
    char timeString[128];
    (void)std::strftime(timeString, sizeof(timeString), format, &calendarTime);

    // Append milliseconds. Example result '2020-06-05 08:20:35.037'.
    char buffer[160];
    int ms = static_cast<int>(tp.tv_usec / 1000);
    (void)std::snprintf(buffer, sizeof(buffer), "%s.%03d", timeString, ms);

    // Return formatted time string.
    return std::string(buffer);
#endif /* _MSC_VER */
}
