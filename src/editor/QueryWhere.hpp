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

#include <QueryFilterSet.hpp>
#include <Range.hpp>
#include <Region.hpp>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Data Query Where. */
class EXPORT_EDITOR QueryWhere
{
public:
    QueryWhere();
    ~QueryWhere();

    void setRegion(const Region &region);
    const Region &region() const { return region_; }

    void setBox(const Box<double> &box);
    const Box<double> &box() const { return region_.box; }

    void setCone(double x, double y, double z, double z2, double angle);
    const Cone<double> &cone() const { return region_.cone; }

    void setCylinder(double ax,
                     double ay,
                     double az,
                     double bx,
                     double by,
                     double bz,
                     double radius);
    const Cylinder<double> &cylinder() const { return region_.cylinder; }

    void setSphere(double x, double y, double z, double radius);
    const Sphere<double> &sphere() const { return region_.sphere; }

    void setElevation(const Range<double> &elevation);
    const Range<double> &elevation() const { return elevation_; }

    void setDescriptor(const Range<double> &descriptor);
    const Range<double> &descriptor() const { return descriptor_; }

    void setDataset(const std::unordered_set<size_t> &list);
    void setDataset(const QueryFilterSet &list);
    const QueryFilterSet &dataset() const { return dataset_; }

    void setClassification(const std::unordered_set<size_t> &list);
    void setClassification(const QueryFilterSet &list);
    const QueryFilterSet &classification() const { return classification_; }
    const std::vector<int> &classificationArray() const
    {
        return classificationArray_;
    }

    void setSegment(const std::unordered_set<size_t> &list);
    void setSegment(const QueryFilterSet &list);
    const QueryFilterSet &segment() const { return segment_; }

private:
    uint32_t filters_;
    Region region_;
    Range<double> elevation_;
    Range<double> descriptor_;
    QueryFilterSet dataset_;
    QueryFilterSet classification_;
    std::vector<int> classificationArray_;
    QueryFilterSet segment_;

    void classificationsToArray();
};

#include <WarningsEnable.hpp>

#endif /* QUERY_WHERE_HPP */
