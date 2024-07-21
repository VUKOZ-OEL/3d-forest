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

/** @file DbhCircle.hpp */

#ifndef DBH_CIRCLE_HPP
#define DBH_CIRCLE_HPP

// Include 3D Forest.
#include <Json.hpp>

/** DBH (Diameter at Breast Height) Circle. */
class DbhCircle
{
public:
    double x{0.0};
    double y{0.0};
    double z{0.0};
    double radius{0.0};
};

inline void toJson(Json &out, const DbhCircle &in)
{
    toJson(out["x"], in.x);
    toJson(out["y"], in.y);
    toJson(out["z"], in.z);
    toJson(out["radius"], in.radius);
}

inline std::string toString(const DbhCircle &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* DBH_CIRCLE_HPP */
