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

/** @file Points.cpp */

#include <Points.hpp>

#define LOG_MODULE_NAME "Points"
#include <Log.hpp>

Points::Points()
{
    points_.create("points.bin");
}

Points::~Points()
{
    try
    {
        points_.close();
    }
    catch (...)
    {
        // empty
    }
}

void Points::clear()
{
    points_.clear();
}

void Points::push_back(const Point &point)
{
    points_.push_back(point);
}

void Points::push_back(Point &&point)
{
    points_.push_back(point);
}

void Points::createIndex()
{
    octree_.initialize(points_);
}

void Points::findRadius(double x,
                        double y,
                        double z,
                        double r,
                        std::vector<size_t> &resultIndices)
{
    octree_.radiusNeighbors<unibn::L2Distance<Point>>({x, y, z},
                                                      r,
                                                      resultIndices);
}

size_t Points::findNN(double x, double y, double z)
{
    int32_t r = octree_.findNeighbor<unibn::L2Distance<Point>>({x, y, z});
    if (r >= 0)
    {
        return static_cast<size_t>(r);
    }

    return SIZE_MAX;
}
