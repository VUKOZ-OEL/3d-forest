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

/** @file ComputeTreeAttributesData.hpp */

#ifndef COMPUTE_TREE_ATTRIBUTES_DATA_HPP
#define COMPUTE_TREE_ATTRIBUTES_DATA_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <TreeAttributes.hpp>
#include <Vector3.hpp>

/** Compute Tree Attributes Data. */
class ComputeTreeAttributesData
{
public:
    /// Tree ID.
    size_t treeId{0};

    /// Collected point coordinates in DBH elevation range.
    std::vector<double> dbhPoints; // x0, y0, z0, x1, y1, z1, ...

    /// Collected point X coordinates in tree base range.
    std::vector<double> xCoordinates;

    /// Collected point Y coordinates in tree base range.
    std::vector<double> yCoordinates;

    /// Height profile.
    std::vector<double> bins;
    double zBoundaryMin;
    double zBoundaryMax;

    /// The Z coordinate of the lowest point.
    double zCoordinateMin{Numeric::max<double>()};

    /// Calculated tree attributes.
    TreeAttributes treeAttributes;
};

inline void toJson(Json &out, const ComputeTreeAttributesData &in)
{
    toJson(out["treeId"], in.treeId);
    toJson(out["treeAttributes"], in.treeAttributes, 1.0);
}

inline std::string toString(const ComputeTreeAttributesData &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* COMPUTE_TREE_ATTRIBUTES_DATA_HPP */
