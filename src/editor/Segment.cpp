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

/** @file Segment.cpp */

#include <Error.hpp>
#include <Segment.hpp>

#define LOG_MODULE_NAME "Segment"
#include <Log.hpp>

Segment::Segment() : id_(0)
{
}

void Segment::set(size_t id,
                  const std::string &label,
                  const Vector3<double> &color)
{
    id_ = id;
    label_ = label;
    color_ = color;
}

void Segment::setLabel(const std::string &label)
{
    label_ = label;
}

void Segment::setColor(const Vector3<double> &color)
{
    color_ = color;
}

void Segment::read(const Json &in)
{
    if (!in.isObject())
    {
        THROW("Segment is not JSON object");
    }

    // ID
    id_ = in["id"].uint32();

    // Label
    if (in.contains("label"))
    {
        label_ = in["label"].string();
    }
    else
    {
        label_ = "";
    }

    // Color
    if (in.contains("color"))
    {
        color_.read(in["color"]);
    }
}

Json &Segment::write(Json &out) const
{
    out["id"] = id_;
    out["label"] = label_;
    color_.write(out["color"]);

    return out;
}
