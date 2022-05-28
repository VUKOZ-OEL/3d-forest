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

/** @file Region.hpp */

#ifndef REGION_HPP
#define REGION_HPP

#include <Box.hpp>

/** Region. */
class Region
{
public:
    /** Region Type. */
    enum Type
    {
        TYPE_NONE,
        TYPE_BOX
    };

    Box<double> box;
    Box<double> boxView;
    Type enabled;

    Region();
    ~Region();

    void clear();

    void read(const Json &in);
    Json &write(Json &out) const;
};

inline Region::Region()
{
    clear();
}

inline Region::~Region()
{
}

inline void Region::clear()
{
    box.clear();
    enabled = TYPE_NONE;
}

inline void Region::read(const Json &in)
{
    box.read(in["box"]);
    if (in["enabled"].string() == "box")
    {
        enabled = TYPE_BOX;
    }
    else
    {
        enabled = TYPE_NONE;
    }
}

inline Json &Region::write(Json &out) const
{
    box.write(out["box"]);
    if (enabled == TYPE_BOX)
    {
        out["enabled"] = "box";
    }
    else
    {
        out["enabled"] = "none";
    }

    return out;
}

#endif /* REGION_HPP */
