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

/** @file QueryWhere.hpp */

#ifndef QUERY_WHERE_HPP
#define QUERY_WHERE_HPP

#include <unordered_set>

#include <Cone.hpp>
#include <ExportEditor.hpp>
#include <Range.hpp>
#include <Sphere.hpp>

/** Data Query Where. */
class EXPORT_EDITOR QueryWhere
{
public:
    QueryWhere();
    ~QueryWhere();

    void setBox(const Box<double> &box);
    const Box<double> &box() const { return box_; }

    void setCone(double x, double y, double z, double z2, double angle);
    const Cone<double> &cone() const { return cone_; }

    void setSphere(double x, double y, double z, double radius);
    const Sphere<double> &sphere() const { return sphere_; }

    void setElevation(const Range<double> &elevation);
    const Range<double> &elevation() const { return elevation_; }

    void setDescriptor(const Range<float> &descriptor);
    const Range<float> &descriptor() const { return descriptor_; }

    void setClassification(const std::unordered_set<size_t> &list);
    const std::vector<int> &classification() const { return classification_; }

    void setLayer(const std::unordered_set<size_t> &list);
    const std::unordered_set<size_t> &layer() const { return layer_; }

private:
    Box<double> box_;
    Cone<double> cone_;
    Sphere<double> sphere_;
    Range<double> elevation_;
    Range<float> descriptor_;
    std::vector<int> classification_;
    std::unordered_set<size_t> layer_;
};

#endif /* QUERY_WHERE_HPP */