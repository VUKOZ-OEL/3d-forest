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

/** @file ComputeConvexHullPlugin.cpp */

// Include 3D Forest.
#include <ComputeConvexHullPlugin.hpp>
#include <ComputeConvexHullWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeConvexHullPlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeConvexHullResources/", name))

ComputeConvexHullPlugin::ComputeConvexHullPlugin()
    : mainWindow_(nullptr),
      pluginWindow_(nullptr)
{
}

void ComputeConvexHullPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Compute",
                              "Compute",
                              tr("Convex Hull"),
                              tr("Compute convex hull"),
                              ICON("convex-hull"),
                              this,
                              SLOT(slotPlugin()),
                              MAIN_WINDOW_MENU_COMPUTE_PRIORITY);
}

void ComputeConvexHullPlugin::slotPlugin()
{
    if (!pluginWindow_)
    {
        pluginWindow_ = new ComputeConvexHullWindow(mainWindow_);
    }

    pluginWindow_->show();
    pluginWindow_->raise();
    pluginWindow_->activateWindow();
}
