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

// Include 3D Forest.
#include <Box.hpp>
#include <Cone.hpp>
#include <Cylinder.hpp>
#include <ExportCore.hpp>
#include <Sphere.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Region. */
class Region
{
public:
    /** Region Shape. */
    enum class Shape
    {
        NONE,
        BOX,
        CONE,
        CYLINDER,
        SPHERE
    };

    Shape shape;

    Box<double> box;
    Cone<double> cone;
    Cylinder<double> cylinder;
    Sphere<double> sphere;

    Box<double> boundary; /// Maximum extent.

    Region();
    ~Region();

    void clear();
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
    shape = Region::Shape::NONE;
}

inline std::string toString(const Region::Shape &in)
{
    switch (in)
    {
        case Region::Shape::BOX:
            return "box";
        case Region::Shape::CONE:
            return "cone";
        case Region::Shape::CYLINDER:
            return "cylinder";
        case Region::Shape::SPHERE:
            return "sphere";
        case Region::Shape::NONE:
        default:
            return "none";
    }
}

inline void fromJson(Region &out, const Json &in)
{
    std::string shape;
    fromJson(shape, in["shape"]);
    if (shape == "box")
    {
        out.shape = Region::Shape::BOX;
        fromJson(out.box, in["box"]);
    }
    else
    {
        out.shape = Region::Shape::NONE;
    }
}

inline void toJson(Json &out, const Region &in)
{
    if (in.shape == Region::Shape::BOX)
    {
        toJson(out["box"], in.box);
    }

    toJson(out["shape"], toString(in.shape));
}

inline std::string toString(const Region &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

inline std::ostream &operator<<(std::ostream &out, const Region &in)
{
    return out << toString(in);
}

#include <WarningsEnable.hpp>

#endif /* REGION_HPP */
