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

/** @file Point.cpp */

#include <cstring>

#include <Endian.hpp>
#include <Point.hpp>

#define LOG_MODULE_NAME "Point"
#include <Log.hpp>

void Point::IO::read(const uint8_t *buffer, Point &point)
{
    point.x = ltohd(&buffer[0]);
    point.y = ltohd(&buffer[8]);
    point.z = ltohd(&buffer[16]);
    point.elevation = ltohd(&buffer[24]);
    point.descriptor = ltohd(&buffer[32]);
    point.dist = ltohd(&buffer[40]);
    point.next = ltoh32(&buffer[48]);
    point.group = ltoh32(&buffer[52]);
    point.status = ltoh32(&buffer[56]);
}

void Point::IO::write(const Point &point, uint8_t *buffer)
{
    htold(&buffer[0], point.x);
    htold(&buffer[8], point.y);
    htold(&buffer[16], point.z);
    htold(&buffer[24], point.elevation);
    htold(&buffer[32], point.descriptor);
    htold(&buffer[40], point.dist);
    htol32(&buffer[48], static_cast<uint32_t>(point.next));
    htol32(&buffer[52], static_cast<uint32_t>(point.group));
    htol32(&buffer[56], point.status);
}

Point::Point(double x_, double y_, double z_) : x(x_), y(y_), z(z_)
{
}
