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

/** @file Points.hpp */

#ifndef POINTS_HPP
#define POINTS_HPP

// 3rd party.
#include <UnibnOctree.hpp>

// Include 3D Forest.
#include <Point.hpp>
#include <VectorFile.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Points. */
class EXPORT_EDITOR Points
{
public:
    // Construct/Copy/Destroy.
    Points();
    ~Points();

    // Capacity.
    bool empty() const { return points_.size() > 0; }
    size_t size() const { return points_.size(); }

    // Element access.
    Point &operator[](size_t pos) { return points_.at(pos); }
    const Point &operator[](size_t pos) const { return points_.at(pos); }
    Point &at(size_t pos) { return points_.at(pos); }
    const Point &at(size_t pos) const { return points_.at(pos); }

    // Modifiers.
    void push_back(const Point &point);
    void push_back(Point &&point);
    void clear();

    // Search.
    void createIndex();
    void findRadius(double x,
                    double y,
                    double z,
                    double r,
                    std::vector<size_t> &resultIndices);
    size_t findNN(double x, double y, double z);

    // I/O
    void exportToFile(const std::string &path) const;

private:
    VectorFile<Point, Point::IO> points_;
    unibn::Octree<Point, VectorFile<Point, Point::IO>> octree_;

    // I/O
    friend void toJson(Json &out, const Points &in);
};

void toJson(Json &out, const Points &in);

#include <WarningsEnable.hpp>

#endif /* POINTS_HPP */
