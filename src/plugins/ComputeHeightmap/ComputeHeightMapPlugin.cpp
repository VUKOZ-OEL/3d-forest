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

/** @file ComputeHeightMapPlugin.cpp */

// Include 3D Forest.
#include <ComputeHeightMapPlugin.hpp>
#include <ComputeHeightMapWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeHeightMapPlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeHeightMapResources/", name))

ComputeHeightMapPlugin::ComputeHeightMapPlugin()
    : mainWindow_(nullptr),
      pluginWindow_(nullptr)
{
}

void ComputeHeightMapPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Compute",
                              "Compute",
                              tr("Height Map"),
                              tr("Compute height map"),
                              ICON("height-map"),
                              this,
                              SLOT(slotPlugin()),
                              MAIN_WINDOW_MENU_COMPUTE_PRIORITY);

    modifier_.initialize(mainWindow_);
}

void ComputeHeightMapPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time.
    if (!pluginWindow_)
    {
        pluginWindow_ = new ComputeHeightMapWindow(mainWindow_, &modifier_);
    }

    pluginWindow_->show();
    pluginWindow_->raise();
    pluginWindow_->activateWindow();
}

bool ComputeHeightMapPlugin::modifierEnabled()
{
    return modifier_.previewEnabled();
}

void ComputeHeightMapPlugin::applyModifier(Page *page)
{
    modifier_.applyModifier(page);
}
