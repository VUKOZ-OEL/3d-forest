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
#include <set>
#include <unordered_set>
#include <vector>

#define LOG_ENABLE 1

#define LOG(msg)                                                               \
    do                                                                         \
    {                                                                          \
        std::cout << __func__ << ": " << msg << "\n";                          \
        std::cout.flush();                                                     \
    } while (false)

#define LOG_MODULE(module, msg)                                                \
    do                                                                         \
    {                                                                          \
        std::cout << (module) << "::" << __func__ << ": " << msg << "\n";      \
        std::cout.flush();                                                     \
    } while (false)

#define LOG_UPDATE_VIEW(module, msg)
//#define LOG_UPDATE_VIEW(module, msg) LOG_MODULE(module, msg)

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
