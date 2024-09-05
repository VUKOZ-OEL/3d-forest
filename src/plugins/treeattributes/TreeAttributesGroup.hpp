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

/** @file TreeAttributesGroup.hpp */

#ifndef TREE_ATTRIBUTES_GROUP_HPP
#define TREE_ATTRIBUTES_GROUP_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

/** Tree Attributes Group. */
class TreeAttributesGroup
{
public:
    size_t segmentId{0};

    std::vector<double> xCoordinates;
    std::vector<double> yCoordinates;
    std::vector<double> dbhPoints; // x0, y0, z0, x1, y1, z1, ...
    double zCoordinateMin{Numeric::max<double>()};

    Vector3<double> position;
    Vector3<double> dbhCenter;
    double dbh{0.0};
};

inline void toJson(Json &out, const TreeAttributesGroup &in)
{
    toJson(out["segmentId"], in.segmentId);
    toJson(out["dbhPointCount"], in.dbhPoints.size() / 3);
    toJson(out["dbhCenter"], in.dbhCenter);
    toJson(out["dbh"], in.dbh);
}

inline std::string toString(const TreeAttributesGroup &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* TREE_ATTRIBUTES_GROUP_HPP */
