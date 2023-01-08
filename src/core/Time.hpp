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

/** @file Time.hpp */

#ifndef TIME_HPP
#define TIME_HPP

#include <cstdint>
#include <string>

#include <ExportCore.hpp>

class Time
{
public:
    /** Get the number of seconds since the Epoch. */
    static double EXPORT_CORE realTime();

    /** Get the number of seconds since the Epoch. */
    static uint64_t EXPORT_CORE realTime64();

    /** Makes the calling thread sleep until milliseconds have elapsed. */
    static void EXPORT_CORE msleep(long milliseconds);

    /** Format current local time to string. */
    static std::string EXPORT_CORE
    strftime(const char *format = "%Y-%m-%d %H:%M:%S");
};

#endif /* TIME_HPP */
