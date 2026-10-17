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

/** @file Point3.hpp */

#ifndef POINT_3_HPP
#define POINT_3_HPP

// Include 3D Forest.
#include <Box.hpp>

/** Point3. */
class Point3
{
public:
    /** Point3 IO. */
    class IO
    {
    public:
        static void read(const uint8_t *buffer, Point3 &point);
        static void write(const Point3 &point, uint8_t *buffer);
    };

    // Class.
    typedef double Type;

    // Data.
    double x;
    double y;
    double z;
    double elevation;
    double descriptor;

    double dist;
    size_t next;

    size_t group;
    uint32_t status;

    // Construct/Copy/Destroy.
    Point3() = default;
    Point3(double x_, double y_, double z_);
};

inline void toJson(Json &out, const Point3 &in)
{
    toJson(out["position"][0], in.x);
    toJson(out["position"][1], in.y);
    toJson(out["position"][2], in.z);
    toJson(out["elevation"], in.elevation);
    toJson(out["descriptor"], in.descriptor);

    toJson(out["group"], in.group);
    toJson(out["status"], in.status);
}

inline std::string toString(const Point3 &in)
{
    Json json;
    toJson(json, in);
    return json.serialize();
}

inline std::ostream &operator<<(std::ostream &out, const Point3 &in)
{
    return out << toString(in);
}

#endif /* POINT_3_HPP */
