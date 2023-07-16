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

/** @file Point.hpp */

#ifndef POINT_HPP
#define POINT_HPP

#include <Box.hpp>

/** Point. */
class Point
{
public:
    /** Point IO. */
    class IO
    {
    public:
        static void read(const uint8_t *buffer, Point &point);
        static void write(const Point &point, uint8_t *buffer);
    };

    // class
    typedef double Type;

    // data
    double x;
    double y;
    double z;
    double elevation;
    double descriptor;

    double dist;
    size_t next;

    size_t group;
    uint32_t status;

    // construct/copy/destroy
    Point() = default;
    Point(double x_, double y_, double z_);
};

inline std::ostream &operator<<(std::ostream &os, const Point &obj)
{
    return os << std::fixed << "<" << obj.status << ", " << obj.group << ", "
              << obj.x << ", " << obj.y << ", " << obj.z << ">"
              << std::defaultfloat;
}

#endif /* POINT_HPP */
