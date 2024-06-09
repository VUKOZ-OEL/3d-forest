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

/** @file Math.hpp */

#ifndef MATH_HPP
#define MATH_HPP

// Include std.
#include <cmath>
#include <limits>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

#define MATH_PI_4 (3.1415926535897932384626433832795 / 4.0)

#define MATH_ATAN_A 0.0776509570923569
#define MATH_ATAN_B -0.287434475393028
#define MATH_ATAN_C (MATH_PI_4 - MATH_ATAN_A - MATH_ATAN_B)

inline double fastatan(double arg)
{
    const double arg2 = arg * arg;
    return ((MATH_ATAN_A * arg2 + MATH_ATAN_B) * arg2 + MATH_ATAN_C) * arg;
}

template <class T> inline T min(const T &a, const T &b)
{
    return (a < b) ? a : b;
}

template <class T> inline T max(const T &a, const T &b)
{
    return (a > b) ? a : b;
}

template <class T> inline void clamp(T &value, const T &min, const T &max)
{
    if (value < min)
    {
        value = min;
    }
    else if (value > max)
    {
        value = max;
    }
}

template <class T> inline void updateRange(const T &value, T &min, T &max)
{
    if (value < min)
    {
        min = value;
    }

    if (value > max)
    {
        max = value;
    }
}

template <class T> inline void normalize(T &value, const T &min, const T &max)
{
    T range = max - min;
    if (range > std::numeric_limits<T>::epsilon())
    {
        value = (value - min) / range;
    }
    else
    {
        value = 0;
    }
}

#include <WarningsEnable.hpp>

#endif /* MATH_HPP */
