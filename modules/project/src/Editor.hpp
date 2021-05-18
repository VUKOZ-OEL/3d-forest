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
    @file Editor.hpp
*/

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <Aabb.hpp>
#include <Camera.hpp>
#include <Database.hpp>
#include <Project.hpp>
#include <mutex>
#include <string>

/** Editor. */
class Editor
{
public:
    Editor();
    ~Editor();

    void lock();
    void unlock();

    void open(const std::string &path);
    void write(const std::string &path);
    void close();

    void updateCamera(const Camera &camera, bool interactionFinished);
    bool loadView();

    bool hasUnsavedChanges() const { return unsavedChanges_; }

    const Project &project() const { return project_; }

    Database &database() { return database_; }
    const Database &database() const { return database_; }

    const Aabb<double> &boundary() const { return database_.boundary(); }
    const Aabb<double> &boundaryView() const
    {
        return database_.boundaryView();
    }

    void setVisibleDataSet(size_t i, bool visible);
    void setVisibleLayer(size_t i, bool visible);
    void setClipFilter(const ClipFilter &clipFilter);

protected:
    std::mutex mutex_;
    Project project_;
    Database database_;
    bool unsavedChanges_;
};

#endif /* EDITOR_HPP */
