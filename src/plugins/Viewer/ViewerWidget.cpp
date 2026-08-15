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

/** @file ViewerWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <VBoxLayout.hpp>
#include <Viewer.hpp>
#include <ViewerWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewerWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ViewerResources/", name))

ViewerWidget::ViewerWidget(Application *app) : app_(app)
{
    viewer_ = new Viewer;

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addWidget(viewer_);

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect([this](const Message &msg) { slotUpdate(msg); });

    slotUpdate({});
}

void ViewerWidget::slotUpdate(const Message &msg)
{
    if (msg.sender() == this)
    {
        return;
    }
}

std::vector<Camera> ViewerWidget::camera(size_t viewportId) const
{
    std::vector<Camera> list;
    return list;
}

std::vector<Camera> ViewerWidget::camera() const
{
    std::vector<Camera> list;
    return list;
}

void ViewerWidget::updateScene()
{
}

void ViewerWidget::resetScene()
{
}

void ViewerWidget::resetSceneView()
{
}
