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

/**
    @file EditorDataSet.cpp
*/

#include <EditorDataSet.hpp>
#include <Error.hpp>
#include <File.hpp>
#include <FileIndexBuilder.hpp>
#include <FileLas.hpp>
#include <iostream>

EditorDataSet::EditorDataSet() : id(0), visible(true)
{
}

EditorDataSet::~EditorDataSet()
{
}

void EditorDataSet::read(const std::string &filePath)
{
    pathUnresolved = filePath;
    setPath(pathUnresolved, File::currentPath() + "\\project");

    read();
}

void EditorDataSet::read(const Json &in, const std::string &projectPath)
{
    if (!in.isObject())
    {
        THROW("Data set is not JSON object");
    }

    // Data set path
    if (!in.containsString("path"))
    {
        THROW("Can't find string 'path' in JSON object");
    }

    pathUnresolved = in["path"].string();
    setPath(pathUnresolved, projectPath);

    // Date Created
    if (in.contains("dateCreated"))
    {
        dateCreated = in["dateCreated"].string();
    }

    // ID
    id = in["id"].uint32();

    // Label
    if (in.contains("label"))
    {
        label = in["label"].string();
    }
    else
    {
        label = "";
    }

    // Visible
    if (in.contains("visible"))
    {
        visible = in["visible"].isTrue();
    }
    else
    {
        visible = true;
    }

    read();
}

Json &EditorDataSet::write(Json &out) const
{
    out["path"] = pathUnresolved;
    out["dateCreated"] = dateCreated;
    out["id"] = id;
    out["label"] = label;
    out["visible"] = visible;

    return out;
}

void EditorDataSet::setPath(const std::string &unresolved,
                            const std::string &projectPath)
{
    // Data set absolute path
    path = unresolved;
    if (!File::isAbsolute(path))
    {
        // Resolve path
        path = File::replaceFileName(projectPath, path);
    }

    // Data set file name
    fileName = File::fileName(path);

    if (!File::exists(path))
    {
        THROW("File '" + path + "' doesn't exist");
    }
}

void EditorDataSet::read()
{
    const std::string pathIndex = FileIndexBuilder::extension(path);
    index.read(pathIndex);

    FileLas las;
    las.open(path);
    las.readHeader();

    if (dateCreated.empty())
    {
        dateCreated = las.header.dateCreated();
    }

    double x1 = las.header.min_x;
    double y1 = las.header.min_y;
    double z1 = las.header.min_z;
    double x2 = las.header.max_x;
    double y2 = las.header.max_y;
    double z2 = las.header.max_z;

    boundary.set(x1, y1, z1, x2, y2, z2);

    las.transformInvert(x1, y1, z1);
    las.transformInvert(x2, y2, z2);
    x1 += las.header.x_offset;
    y1 += las.header.y_offset;
    z1 += las.header.z_offset;
    x2 += las.header.x_offset;
    y2 += las.header.y_offset;
    z2 += las.header.z_offset;
    boundaryView.set(x1, y1, z1, x2, y2, z2);
}
