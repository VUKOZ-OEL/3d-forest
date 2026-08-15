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

/** @file ViewerPlugin.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <ViewerPlugin.hpp>
#include <ViewerWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewerPlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ViewerResources/", name))

void ViewerPlugin::initialize(Application *app)
{
    LOG_DEBUG(<< "Start initializing viewer plugin.");
    app_ = app;
    widget_ = new ViewerWidget(app_);
    app_->setViewer(widget_);
    LOG_DEBUG(<< "Finished initializing viewer plugin.");
}

void ViewerPlugin::release()
{
    if (widget_)
    {
        app_->removeViewer(widget_);
    }

    delete this;
}

std::vector<Camera> ViewerPlugin::camera(size_t viewportId) const
{
    if (widget_)
    {
        return widget_->camera(viewportId);
    }

    return std::vector<Camera>();
}

std::vector<Camera> ViewerPlugin::camera() const
{
    if (widget_)
    {
        return widget_->camera();
    }

    return std::vector<Camera>();
}

void ViewerPlugin::updateScene()
{
    if (widget_)
    {
        return widget_->updateScene();
    }
}

void ViewerPlugin::resetScene()
{
    if (widget_)
    {
        return widget_->resetScene();
    }
}

void ViewerPlugin::resetSceneView()
{
    if (widget_)
    {
        return widget_->resetSceneView();
    }
}
