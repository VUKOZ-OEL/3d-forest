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

/** @file DensityPlugin.cpp */

#include <DensityPlugin.hpp>
#include <DensityWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#define LOG_MODULE_NAME "DensityPlugin"
#define ICON(name) (ThemeIcon(":/density/", name))

DensityPlugin::DensityPlugin() : mainWindow_(nullptr), pluginWindow_(nullptr)
{
}

void DensityPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Utilities",
                              "Utilities",
                              tr("Density"),
                              tr("Compute density around each point"),
                              ICON("density"),
                              this,
                              SLOT(slotPlugin()));
}

void DensityPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time
    if (!pluginWindow_)
    {
        pluginWindow_ = new DensityWindow(mainWindow_);
    }

    pluginWindow_->show();
    pluginWindow_->raise();
    pluginWindow_->activateWindow();
}
