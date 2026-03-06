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

/** @file TreeAttributes.cpp */

// Include std.
#include <unordered_set>

// Include 3D Forest.
#include <TreeAttributes.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeAttributes"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

bool TreeAttributes::isValid() const
{
    return isHeightValid() && isPositionValid();
}

bool TreeAttributes::isPositionValid() const
{
    return position != Vector3<double>();
}

bool TreeAttributes::isHeightValid() const
{
    return height > 0.0;
}

bool TreeAttributes::isDbhValid() const
{
    return dbh > 0.0 && dbhPosition != Vector3<double>();
}

double TreeAttributes::number(const std::string &key, double defaultValue) const
{
    const auto it = attributes.find(key);
    if (it == attributes.end())
    {
        return defaultValue;
    }

    if (auto v = std::any_cast<double>(&it->second))
    {
        return *v;
    }

    return defaultValue;
}

void toJson(Json &out, const TreeAttributes &in, double scale)
{
    double scale2 = scale * scale;
    double scale3 = scale * scale * scale;

    toJson(out["position"], in.position * scale);
    toJson(out["height"], in.height * scale);
    toJson(out["surfaceAreaProjection"], in.surfaceAreaProjection * scale2);
    toJson(out["surfaceArea"], in.surfaceArea * scale2);
    toJson(out["volume"], in.volume * scale3);
    toJson(out["dbhPosition"], in.dbhPosition * scale);
    toJson(out["dbhNormal"], in.dbhNormal);
    toJson(out["dbh"], in.dbh * scale);

    toJson(out["crownCenter"], in.crownCenter * scale);
    toJson(out["crownStartHeight"], in.crownStartHeight * scale);
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

    toJson(out["crownVoxelSize"], in.crownVoxelSize * scale);

    for (const auto &it : in.attributes)
    {
        toJson(out[it.first], it.second);
    }
}

void fromJson(TreeAttributes &out, const Json &in, double scale)
{
    if (!in.typeObject())
    {
        return;
    }

    std::unordered_set<std::string> knownTags = {
        "position", "height",
        "surfaceAreaProjection", "surfaceArea", "volume",
        "dbhPosition", "dbhNormal", "dbh",
        "crownCenter", "crownStartHeight",
        "crownVoxelCountPerMeters", "crownVoxelCount", "crownVoxelCountShared",
        "crownVoxelSize" };

    for (const auto &it : in.object())
    {
        if (knownTags.count(it.first) == 0)
        {
            std::any value;
            fromJson(value, it.second);
            out.attributes[it.first] = value;

            if (value.type() == typeid(double))
            {
                LOG_DEBUG(<< "Read custom tag <" << it.first << "> value <" <<
                          std::any_cast<double>(value) << ">");
            }
            else
            {
                LOG_DEBUG(<< "Read custom tag <" << it.first << ">");
            }

            continue;
        }
    }

    fromJson(out.position, in, "position");
    out.position = out.position * scale;

    fromJson(out.height, in, "height");
    out.height *= scale;

    fromJson(out.surfaceAreaProjection, in, "surfaceAreaProjection");
    out.surfaceAreaProjection = out.surfaceAreaProjection * (scale * scale);

    fromJson(out.surfaceArea, in, "surfaceArea");
    out.surfaceArea = out.surfaceArea * (scale * scale);

    fromJson(out.volume, in, "volume");
    out.volume = out.volume * (scale * scale * scale);

    fromJson(out.dbhPosition, in, "dbhPosition");
    out.dbhPosition = out.dbhPosition * scale;

    fromJson(out.dbhNormal, in, "dbhNormal", Vector3<double>(0.0, 0.0, 1.0));

    fromJson(out.dbh, in, "dbh");
    out.dbh *= scale;

    fromJson(out.crownCenter, in, "crownCenter");
    out.crownCenter = out.crownCenter * scale;

    fromJson(out.crownStartHeight, in, "crownStartHeight");
    out.crownStartHeight *= scale;

    fromJson(out.crownVoxelCountPerMeters, in, "crownVoxelCountPerMeters");

    fromJson(out.crownVoxelCount, in, "crownVoxelCount");

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

    fromJson(out.crownVoxelSize, in, "crownVoxelSize");
    out.crownVoxelSize *= scale;
}

std::string toString(const TreeAttributes &in)
{
    Json json;
    toJson(json, in, 1.0);
    return json.serialize(0);
}
