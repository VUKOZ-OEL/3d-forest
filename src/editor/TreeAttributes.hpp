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
    /// Calculated tree position from X and Y coordinates in tree base range.
    Vector3<double> position;

    /// Calculated tree height.
    double height{0.0};

    /// Calculated crown start.
    double crownStartHeight{0.0};

    /// Crown voxel counts per each meter.
    std::vector<size_t> crownVoxelCountPerMeters;

    /// Crown voxel count.
    size_t crownVoxelCount{0};

    /// Shared crown voxel count [other tree id : count].
    std::map<size_t, size_t> crownVoxelCountShared;

    /// Crown voxel size.
    double crownVoxelSize{0.0};

    /// Area of tree projection from top view.
    double surfaceAreaProjection{0.0};

    /// Tree surface area.
    double surfaceArea{0.0};

    /// Tree volume.
    double volume{0.0};

    /// The position of calculated DBH circle.
    Vector3<double> dbhPosition;

    /// The normal vector of calculated DBH circle.
    Vector3<double> dbhNormal;

    /// Calculated DBH (Diameter at Breast Height) value.
    double dbh{0.0};

    /// Validate position and height.
    bool isValid() const;

    /// Validate position.
    bool isPositionValid() const;

    /// Validate height.
    bool isHeightValid() const;

    /// Validate DBH.
    bool isDbhValid() const;
};

inline bool TreeAttributes::isValid() const
{
    return isHeightValid() && isPositionValid();
}

inline bool TreeAttributes::isPositionValid() const
{
    return position != Vector3<double>();
}

inline bool TreeAttributes::isHeightValid() const
{
    return height > 0.0;
}

inline bool TreeAttributes::isDbhValid() const
{
    return dbh > 0.0 && dbhPosition != Vector3<double>();
}

inline void toJson(Json &out, const TreeAttributes &in)
{
    toJson(out["position"], in.position);
    toJson(out["height"], in.height);
    toJson(out["surfaceAreaProjection"], in.surfaceAreaProjection);
    toJson(out["surfaceArea"], in.surfaceArea);
    toJson(out["volume"], in.volume);
    toJson(out["dbhPosition"], in.dbhPosition);
    toJson(out["dbhNormal"], in.dbhNormal);
    toJson(out["dbh"], in.dbh);

    toJson(out["crownStartHeight"], in.crownStartHeight);
    toJson(out["crownVoxelCountPerMeters"], in.crownVoxelCountPerMeters);
    toJson(out["crownVoxelCount"], in.crownVoxelCount);

    Json &list = out["crownVoxelCountShared"];
    size_t idx = 0;
    for (const auto &it : in.crownVoxelCountShared)
    {
        toJson(list[idx]["treeId"], it.first);
        toJson(list[idx]["count"], it.second);
        idx++;
    }

    toJson(out["crownVoxelSize"], in.crownVoxelSize);
}

inline void fromJson(TreeAttributes &out, const Json &in)
{
    fromJson(out.position, in["position"]);
    fromJson(out.height, in["height"]);
    fromJson(out.surfaceAreaProjection, in["surfaceAreaProjection"]);
    fromJson(out.surfaceArea, in["surfaceArea"]);
    fromJson(out.volume, in["volume"]);
    fromJson(out.dbhPosition, in["dbhPosition"]);
    if (in.contains("dbhNormal"))
    {
        fromJson(out.dbhNormal, in["dbhNormal"]);
    }
    else
    {
        out.dbhNormal.set(0.0, 0.0, 1.0);
    }
    fromJson(out.dbh, in["dbh"]);

    if (in.contains("crownStartHeight"))
    {
        fromJson(out.crownStartHeight, in["crownStartHeight"]);
    }

    if (in.contains("crownVoxelCountPerMeters"))
    {
        fromJson(out.crownVoxelCountPerMeters, in["crownVoxelCountPerMeters"]);
    }

    if (in.contains("crownVoxelCount"))
    {
        fromJson(out.crownVoxelCount, in["crownVoxelCount"]);
    }

    out.crownVoxelCountShared.clear();
    if (in.contains("crownVoxelCountShared") &&
        in["crownVoxelCountShared"].typeArray())
    {
        const auto &list = in["crownVoxelCountShared"].array();
        for (size_t i = 0; i < list.size(); i++)
        {
            size_t k;
            size_t v;
            const auto &item = list[i];
            fromJson(k, item["treeId"]);
            fromJson(v, item["count"]);
            out.crownVoxelCountShared[k] = v;
        }
    }

    if (in.contains("crownVoxelSize"))
    {
        fromJson(out.crownVoxelSize, in["crownVoxelSize"]);
    }
}

inline std::string toString(const TreeAttributes &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

#endif /* TREE_ATTRIBUTES_HPP */
