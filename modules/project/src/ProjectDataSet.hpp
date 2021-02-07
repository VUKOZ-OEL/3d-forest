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
    @file ProjectDataSet.hpp
*/

#ifndef PROJECT_DATA_SET_HPP
#define PROJECT_DATA_SET_HPP

#include <Json.hpp>
#include <string>

/** Project Data Set. */
class ProjectDataSet
{
public:
    // Stored
    std::string pathUnresolved;
    std::string dateCreated; //! Note: Inconsistent with LAS, shared projects
    std::string label;       //! Note: Inconsistent with LAS, shared projects
    size_t id;
    bool visible;

    // Derived
    std::string path;
    std::string fileName;

    ProjectDataSet();
    ~ProjectDataSet();

    void read(const Json &in, const std::string &projectPath);
    Json &write(Json &out) const;

    void setPath(const std::string &unresolved, const std::string &projectPath);
};

#endif /* PROJECT_DATA_SET_HPP */
