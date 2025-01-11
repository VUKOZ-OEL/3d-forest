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

/** @file ComputeConvexHullData.hpp */

#ifndef COMPUTE_CONVEX_HULL_DATA_HPP
#define COMPUTE_CONVEX_HULL_DATA_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <TreeAttributes.hpp>
#include <Vector3.hpp>

/** Compute Convex Hull Data. */
class ComputeConvexHullData
{
public:
    /// Tree ID.
    size_t treeId{0};

    /// Collected point coordinates
    std::vector<double> points; // x0, y0, z0, x1, y1, z1, ...
};

inline void toJson(Json &out, const ComputeConvexHullData &in)
{
    toJson(out["treeId"], in.treeId);
}

inline std::string toString(const ComputeConvexHullData &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_CONVEX_HULL_DATA_HPP */
