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

/** @file QueryWhere.cpp */

// Include 3D Forest.
#include <QueryWhere.hpp>

// Include local.
#define LOG_MODULE_NAME "QueryWhere"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

QueryWhere::QueryWhere()
{
}

QueryWhere::~QueryWhere()
{
}

void QueryWhere::clear()
{
    LOG_DEBUG_UPDATE(<< "Clear.");

    region_.clear();
    elevation_.clear();
    descriptor_.clear();
    intensity_.clear();
    dataset_.clear();
    classification_.clear();
    classificationArray_.clear();
    segment_.clear();
    speciesFilter_.clear();
}

void QueryWhere::setDataset(const QueryFilterSet &list)
{
    dataset_ = list;
}

void QueryWhere::setDataset(const std::unordered_set<size_t> &list)
{
    dataset_.setFilter(list);
}

void QueryWhere::setRegion(const Region &region)
{
    region_ = region;
}

void QueryWhere::setBox(const Box<double> &box)
{
    region_.box = box;
    region_.shape = Region::Shape::BOX;
}

void QueryWhere::setCone(double x, double y, double z, double z2, double angle)
{
    region_.cone.set(x, y, z, z2, angle);
    region_.shape = Region::Shape::CONE;
}

void QueryWhere::setCylinder(double ax,
                             double ay,
                             double az,
                             double bx,
                             double by,
                             double bz,
                             double radius)
{
    region_.cylinder.set(ax, ay, az, bx, by, bz, radius);
    region_.shape = Region::Shape::CYLINDER;
}

void QueryWhere::setSphere(double x, double y, double z, double radius)
{
    region_.sphere.set(x, y, z, radius);
    region_.shape = Region::Shape::SPHERE;
}

void QueryWhere::setElevation(const Range<double> &elevation)
{
    elevation_ = elevation;
}

void QueryWhere::setDescriptor(const Range<double> &descriptor)
{
    descriptor_ = descriptor;
}

void QueryWhere::setIntensity(const Range<double> &intensity)
{
    intensity_ = intensity;
}

void QueryWhere::setClassification(const QueryFilterSet &list)
{
    classification_ = list;
    classificationsToArray();
}

void QueryWhere::setClassification(const std::unordered_set<size_t> &list)
{
    classification_.setFilter(list);
    classificationsToArray();
}

void QueryWhere::classificationsToArray()
{
    classificationArray_.resize(256);

    for (size_t i = 0; i < classificationArray_.size(); i++)
    {
        classificationArray_[i] = 0;
    }

    for (auto const &it : classification_.filter())
    {
        if (it < classificationArray_.size())
        {
            classificationArray_[it] = 1;
        }
    }
}

void QueryWhere::setSegment(const QueryFilterSet &list)
{
    segment_ = list;
}

void QueryWhere::setSegment(const std::unordered_set<size_t> &list)
{
    segment_.setFilter(list);
}

void QueryWhere::setSpecies(const QueryFilterSet &list)
{
    speciesFilter_ = list;
}

void QueryWhere::setSpecies(const std::unordered_set<size_t> &list)
{
    speciesFilter_.setFilter(list);
}

void QueryWhere::setManagementStatus(const QueryFilterSet &list)
{
    managementStatusFilter_ = list;
}

void QueryWhere::setManagementStatus(const std::unordered_set<size_t> &list)
{
    managementStatusFilter_.setFilter(list);
}
