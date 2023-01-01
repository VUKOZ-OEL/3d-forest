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

#include <QueryWhere.hpp>

QueryWhere::QueryWhere()
{
}

QueryWhere::~QueryWhere()
{
}

void QueryWhere::setBox(const Box<double> &box)
{
    box_ = box;
}

void QueryWhere::setCone(double x, double y, double z, double z2, double angle)
{
    cone_.set(x, y, z, z2, angle);
}

void QueryWhere::setSphere(double x, double y, double z, double radius)
{
    sphere_.set(x, y, z, radius);
}

void QueryWhere::setElevation(const Range<double> &elevation)
{
    elevation_ = elevation;
}

void QueryWhere::setDescriptor(const Range<float> &descriptor)
{
    descriptor_ = descriptor;
}

void QueryWhere::setDataset(const QueryFilterSet &list)
{
    dataset_ = list;
}

void QueryWhere::setDataset(const std::unordered_set<size_t> &list)
{
    dataset_.setFilter(list);
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

void QueryWhere::setLayer(const QueryFilterSet &list)
{
    layer_ = list;
}

void QueryWhere::setLayer(const std::unordered_set<size_t> &list)
{
    layer_.setFilter(list);
}
