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
    @file ProjectDataSet.cpp
*/

#include <Error.hpp>
#include <File.hpp>
#include <ProjectDataSet.hpp>

ProjectDataSet::ProjectDataSet()
{
}

ProjectDataSet::~ProjectDataSet()
{
}

void ProjectDataSet::read(const Json &in, const std::string &projectPath)
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
    if (!File::exists(path))
    {
        THROW("File '" + path + "' doesn't exist");
    }

    // Date Created
    if (in.contains("dateCreated"))
    {
        dateCreated = in["dateCreated"].string();
    }
    else
    {
        // TBD
        // LasFile las;
        // las.open(path);
        // las.readHeader();
        // dateCreated = las.dateCreated();
        dateCreated = "";
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
}

Json &ProjectDataSet::write(Json &out) const
{
    out["path"] = pathUnresolved;
    out["dateCreated"] = dateCreated;
    out["id"] = id;
    out["label"] = label;
    out["visible"] = visible;

    return out;
}

void ProjectDataSet::setPath(const std::string &unresolved,
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
}
