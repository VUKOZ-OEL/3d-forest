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

/** @file EditorDataSet.cpp */

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

void EditorDataSet::read(const std::string &filePath,
                         const std::string &projectPath)
{
    pathUnresolved = filePath;
    setPath(pathUnresolved, projectPath);

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

    // Read
    read();

    // Transformation
    if (in.contains("translation"))
    {
        translation.read(in["translation"]);
    }

    if (in.contains("scaling"))
    {
        scaling.read(in["scaling"]);
    }

    updateBoundary();
}

Json &EditorDataSet::write(Json &out) const
{
    out["path"] = pathUnresolved;
    out["dateCreated"] = dateCreated;
    out["id"] = id;
    out["label"] = label;
    out["visible"] = visible;

    translation.write(out["translation"]);
    scaling.write(out["scaling"]);

    return out;
}

void EditorDataSet::setPath(const std::string &unresolved,
                            const std::string &projectPath)
{
    // Data set absolute path
    path = resolvePath(unresolved, projectPath);

    // Data set file name
    fileName = File::fileName(path);
}

std::string EditorDataSet::resolvePath(const std::string &unresolved,
                                       const std::string &projectPath)
{
    std::string rval;

    rval = unresolved;
    if (!File::isAbsolute(rval))
    {
        // Resolve path
        rval = File::replaceFileName(projectPath, rval);
    }

    if (!File::exists(rval))
    {
        THROW("File '" + rval + "' doesn't exist");
    }

    return rval;
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

    translationFile.set(las.header.x_offset,
                        las.header.y_offset,
                        las.header.z_offset);

    translation = translationFile;

    scalingFile.set(las.header.x_scale_factor,
                    las.header.y_scale_factor,
                    las.header.z_scale_factor);

    scaling.set(1.0, 1.0, 1.0);

    boundaryFile = index.boundary();
    updateBoundary();
}

void EditorDataSet::updateBoundary()
{
    boundary = boundaryFile;
    boundary.translate(translation);

    boundaryView = boundary;
}
