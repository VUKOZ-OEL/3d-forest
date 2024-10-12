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

/** @file TreeAttributesData.hpp */

#ifndef TREE_ATTRIBUTES_DATA_HPP
#define TREE_ATTRIBUTES_DATA_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <TreeAttributes.hpp>
#include <Vector3.hpp>

/** Tree Attributes Data. */
class TreeAttributesData
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

    /// The Z coordinate of the lowest point.
    double zCoordinateMin{Numeric::max<double>()};

    /// The elevation value of the highest point.
    double elevationMax{Numeric::min<double>()};

    /// Calculated tree attributes.
    TreeAttributes treeAttributes;
};

inline void toJson(Json &out, const TreeAttributesData &in)
{
    toJson(out["treeId"], in.treeId);
    toJson(out["treeAttributes"], in.treeAttributes);
}

inline std::string toString(const TreeAttributesData &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* TREE_ATTRIBUTES_DATA_HPP */
