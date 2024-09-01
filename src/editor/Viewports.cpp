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

/** @file Viewports.cpp */

// Include 3D Forest.
#include <Viewports.hpp>

// Include local.
#define LOG_MODULE_NAME "Viewports"
#include <Log.hpp>

Viewports::Viewports() : activeViewport_(0)
{
}

Viewports::~Viewports()
{
}

void Viewports::resize(Editor *editor, size_t n)
{
    size_t i = viewports_.size();

    while (i < n)
    {
        std::shared_ptr<Query> viewport = std::make_shared<Query>(editor);
        viewports_.push_back(viewport);
        i++;
    }

    while (n < i)
    {
        viewports_.pop_back();
        i--;
    }
}

void Viewports::clearContent()
{
    for (auto &it : viewports_)
    {
        it->clear();
    }
}

void Viewports::applyWhereToAll()
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (i != activeViewport_)
        {
            viewports_[i]->setWhere(where());
        }
    }
}

void Viewports::applyCamera(const std::vector<Camera> &cameraList)
{
    for (size_t i = 0; i < cameraList.size(); i++)
    {
        if (cameraList[i].viewportId < viewports_.size())
        {
            LOG_DEBUG(<< "Apply camera <" << cameraList[i] << ">.");
            viewports_[cameraList[i].viewportId]->applyCamera(cameraList[i]);
        }
        else
        {
            LOG_DEBUG(<< "Skip camera <" << cameraList[i] << ">.");
        }
    }
}

void Viewports::setState(Page::State state)
{
    for (auto &it : viewports_)
    {
        it->setState(state);
    }
}

bool Viewports::nextState()
{
    bool continuing = false;

    for (auto &it : viewports_)
    {
        if (it->nextState())
        {
            continuing = true;
        }
    }

    return continuing;
}
