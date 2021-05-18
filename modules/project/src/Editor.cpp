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
    @file Editor.cpp
*/

#include <Editor.hpp>

Editor::Editor() : unsavedChanges_(false)
{
}

Editor::~Editor()
{
}

void Editor::lock()
{
    mutex_.lock();
}

void Editor::unlock()
{
    mutex_.unlock();
}

void Editor::open(const std::string &path)
{
    project_.read(path);

    for (size_t i = 0; i < project_.dataSetSize(); i++)
    {
        const ProjectDataSet &ds = project_.dataSet(i);
        database_.addDataSet(ds.id, ds.path, ds.visible);
    }
}

void Editor::write(const std::string &path)
{
    project_.write(path);
    unsavedChanges_ = false;
}

void Editor::close()
{
    project_.clear();
    database_.clear();
    unsavedChanges_ = false;
}

void Editor::updateCamera(const Camera &camera, bool interactionFinished)
{
    database_.updateCamera(camera, interactionFinished);
}

bool Editor::loadView()
{
    return database_.loadView();
}

void Editor::setVisibleDataSet(size_t i, bool visible)
{
    project_.setVisibleDataSet(i, visible);
    unsavedChanges_ = true;
}

void Editor::setVisibleLayer(size_t i, bool visible)
{
    project_.setVisibleLayer(i, visible);
    unsavedChanges_ = true;
}

void Editor::setClipFilter(const ClipFilter &clipFilter)
{
    project_.setClipFilter(clipFilter);
    unsavedChanges_ = true;
}
