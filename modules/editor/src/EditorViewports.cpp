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

/** @file EditorViewports.cpp */

#include <EditorViewports.hpp>

EditorViewports::EditorViewports()
{
}

EditorViewports::~EditorViewports()
{
}

void EditorViewports::resize(EditorDatabase *database, size_t n)
{
    size_t i = viewports_.size();

    while (i < n)
    {
        std::shared_ptr<EditorQuery> viewport =
            std::make_shared<EditorQuery>(database);
        viewports_.push_back(viewport);
        i++;
    }

    while (n < i)
    {
        viewports_.pop_back();
        i--;
    }
}

void EditorViewports::clearContent()
{
    for (auto &it : viewports_)
    {
        it->clear();
    }
}

void EditorViewports::selectBox(const Box<double> &box)
{
    for (auto &it : viewports_)
    {
        it->selectBox(box);
    }
}

void EditorViewports::selectCamera(size_t viewport, const Camera &camera)
{
    viewports_[viewport]->selectCamera(camera);
}

void EditorViewports::setState(EditorPage::State state)
{
    for (auto &it : viewports_)
    {
        it->setState(state);
    }
}

bool EditorViewports::nextState()
{
    bool rval = true;

    for (auto &it : viewports_)
    {
        if (!it->nextState())
        {
            rval = false;
        }
    }

    return rval;
}
