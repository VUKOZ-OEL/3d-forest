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
    @file ProjectFile.cpp
*/

#include <Error.hpp>
#include <LasFile.hpp>
#include <ProjectFile.hpp>
#include <filesystem>
#include <iostream>

ProjectFile::ProjectFile()
{
}

ProjectFile::~ProjectFile()
{
}

void ProjectFile::read(const Json &json)
{
    if (!json.containsString("pointCloudPath"))
    {
        THROW("Can't find string 'pointCloudPath'");
    }
    path = json["pointCloudPath"].string();

    std::filesystem::path fsPath(path);
    if (!std::filesystem::exists(fsPath))
    {
        THROW("File '" + path + "' doesn't exist");
    }

    fileName = fsPath.filename().string();

    // Data file
    // LasFile las;
    // las.open(file.fileName);

    // Override timestamp
    if (json.containsString("timestamp"))
    {
        timestamp = json["timestamp"].string();
    }
}

