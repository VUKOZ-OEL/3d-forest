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

/** @file ClipFilter.hpp */

#ifndef CLIP_FILTER_HPP
#define CLIP_FILTER_HPP

#include <Aabb.hpp>

/** Clip Filter. */
class ClipFilter
{
public:
    /** Clip Filter Type. */
    enum Type
    {
        TYPE_NONE,
        TYPE_BOX
    };

    Aabb<double> box;
    Aabb<double> boxView;
    Type enabled;

    ClipFilter();
    ~ClipFilter();

    void clear();

    void read(const Json &in);
    Json &write(Json &out) const;
};

inline ClipFilter::ClipFilter()
{
    clear();
}

inline ClipFilter::~ClipFilter()
{
}

inline void ClipFilter::clear()
{
    box.clear();
    enabled = TYPE_NONE;
}

inline void ClipFilter::read(const Json &in)
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

inline Json &ClipFilter::write(Json &out) const
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

#endif /* CLIP_FILTER_HPP */
