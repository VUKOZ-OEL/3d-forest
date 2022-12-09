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

void QueryWhere::setClassification(const std::unordered_set<size_t> &list)
{
    if (list.empty())
    {
        classification_.clear();
    }
    else
    {
        size_t n = 256;

        classification_.resize(n);

        for (size_t i = 0; i < n; i++)
        {
            classification_[i] = 0;
        }

        for (auto const &it : list)
        {
            classification_[it] = 1;
        }
    }
}

void QueryWhere::setLayer(const std::unordered_set<size_t> &list)
{
    layer_ = list;
}
