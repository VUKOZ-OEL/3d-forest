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
//#include <MeshNode.hpp>

Editor::Editor() : unsavedChanges_(false)
{
}

Editor::~Editor()
{
}

void Editor::open(const std::string &path)
{
    project_.read(path);

    for (size_t i = 0; i < project_.dataSetSize(); i++)
    {
        const ProjectDataSet &dataSet = project_.dataSet(i);
        database_.openDataSet(dataSet.id, dataSet.path);
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
    boundary_.clear();
    unsavedChanges_ = false;
}

void Editor::updateView()
{
    database_.updateView();
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
