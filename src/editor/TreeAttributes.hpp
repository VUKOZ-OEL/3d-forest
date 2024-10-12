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

/** @file TreeAttributes.hpp */

#ifndef TREE_ATTRIBUTES_HPP
#define TREE_ATTRIBUTES_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

/** Tree Attributes. */
class TreeAttributes
{
public:
    /** Tree Attributes Status. */
    enum class Status
    {
        UNKNOWN,
        VALID,
        INVALID
    };

    /// Calculated tree position from X and Y coordinates in tree base range.
    Vector3<double> position;

    /// Calculated tree height.
    double height{0.0};

    /// The position of calculated DBH circle.
    Vector3<double> dbhPosition;

    /// Calculated DBH (Diameter at Breast Height) value.
    double dbh{0.0};

    /// Result of tree attributes calculation.
    TreeAttributes::Status status{TreeAttributes::Status::UNKNOWN};
};

inline std::string toString(const TreeAttributes::Status &in)
{
    switch (in)
    {
        case TreeAttributes::Status::VALID:
            return "Valid";
        case TreeAttributes::Status::INVALID:
            return "Invalid";
        case TreeAttributes::Status::UNKNOWN:
        default:
            return "Unknown";
    }
}

inline void fromString(TreeAttributes::Status &out, const std::string &in)
{
    if (in == "Valid")
    {
        out = TreeAttributes::Status::VALID;
    }
    else if (in == "Invalid")
    {
        out = TreeAttributes::Status::INVALID;
    }
    else
    {
        out = TreeAttributes::Status::UNKNOWN;
    }
}

inline void toJson(Json &out, const TreeAttributes::Status &in)
{
    out = toString(in);
}

inline void fromJson(TreeAttributes::Status &out, const Json &in)
{
    fromString(out, in.string());
}

inline void toJson(Json &out, const TreeAttributes &in)
{
    toJson(out["position"], in.position);
    toJson(out["height"], in.height);
    toJson(out["dbhPosition"], in.dbhPosition);
    toJson(out["dbh"], in.dbh);
    toJson(out["status"], in.status);
}

inline void fromJson(TreeAttributes &out, const Json &in)
{
    fromJson(out.position, in["position"]);
    fromJson(out.height, in["height"]);
    fromJson(out.dbhPosition, in["dbhPosition"]);
    fromJson(out.dbh, in["dbh"]);
    fromJson(out.status, in["status"]);
}

inline std::string toString(const TreeAttributes &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* TREE_ATTRIBUTES_HPP */
