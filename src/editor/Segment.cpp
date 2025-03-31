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

/** @file Segment.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <Segment.hpp>

// Include local.
#define LOG_MODULE_NAME "Segment"
#include <Log.hpp>

Segment::Segment()
{
}

void Segment::exportMeshList(const std::string &projectFilePath) const
{
    std::string extId = "." + toString(id) + ".ply";
    std::string pathId = File::replaceExtension(projectFilePath, extId);

    for (const auto &it : meshList)
    {
        std::string extMesh = "." + it.first + ".ply";
        std::string pathMesh = File::replaceExtension(pathId, extMesh);
        it.second.exportPLY(pathMesh);
    }
}

void Segment::importMeshList(const std::string &projectFilePath)
{
}

void fromJson(Segment &out, const Json &in)
{
    fromJson(out.id, in["id"]);
    fromJson(out.label, in["label"]);
    fromJson(out.color, in["color"]);

    fromJson(out.boundary, in["boundary"]);
    fromJson(out.treeAttributes, in["treeAttributes"]);
}

void toJson(Json &out, const Segment &in)
{
    toJson(out["id"], in.id);
    toJson(out["label"], in.label);
    toJson(out["color"], in.color);

    toJson(out["boundary"], in.boundary);
    toJson(out["treeAttributes"], in.treeAttributes);
}

std::ostream &operator<<(std::ostream &out, const Segment &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
