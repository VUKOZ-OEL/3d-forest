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
    @file Project.cpp
*/

#include <Error.hpp>
#include <Project.hpp>

Project::Project()
{
}

Project::~Project()
{
}

void Project::open(const std::string &path)
{
    close();

    path_ = path;

    Json project;
    project.read(path_);

    try
    {
        if (project.isObject())
        {
            if (project.containsString("projectName"))
            {
                projectName_ = project["projectName"].string();
            }

            if (project.containsArray("snapshots"))
            {
                for (auto const &it : project["snapshots"].array())
                {
                    readFile(it);
                }
            }
        }
        else
        {
            THROW("Project file is not in JSON object");
        }
    }
    catch (std::exception &e)
    {
        close();
        throw;
    }
}

void Project::close()
{
    path_ = "";
    projectName_ = "";
    files_.clear();
}

void Project::readFile(const Json &json)
{
    if (!json.isObject())
    {
        THROW("A snapshot in project file is not in JSON object");
    }

    std::shared_ptr<ProjectFile> file = std::make_shared<ProjectFile>();
    file->read(json);
    files_.push_back(file);
}
