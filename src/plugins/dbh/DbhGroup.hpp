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

/** @file DbhGroup.hpp */

#ifndef DBH_GROUP_HPP
#define DBH_GROUP_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

/** DBH (Diameter at Breast Height) Group. */
class DbhGroup
{
public:
    size_t segmentId{0};
    std::vector<double> points; // x0, y0, z0, x1, y1, z1, ...

    Vector3<double> center;
    double radius{0.0};
};

inline void toJson(Json &out, const DbhGroup &in)
{
    toJson(out["segmentId"], in.segmentId);
    toJson(out["pointCount"], in.points.size() / 3);
    toJson(out["center"], in.center);
    toJson(out["radius"], in.radius);
}

inline std::string toString(const DbhGroup &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* DBH_GROUP_HPP */
