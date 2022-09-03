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

/** @file HeightMapPlugin.cpp */

#include <HeightMapPlugin.hpp>
#include <HeightMapWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/heightmap/", name))

HeightMapPlugin::HeightMapPlugin() : mainWindow_(nullptr), dockWindow_(nullptr)
{
}

void HeightMapPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Modifiers",
                              "Modifiers",
                              tr("Height Map"),
                              tr("Compute height map"),
                              ICON("height_map"),
                              this,
                              SLOT(slotPlugin()));

    modifier_.initialize(mainWindow_);
}

void HeightMapPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time
    if (!dockWindow_)
    {
        dockWindow_ = new HeightMapWindow(mainWindow_, &modifier_);
    }

    dockWindow_->show();
    dockWindow_->raise();
    dockWindow_->activateWindow();
}

bool HeightMapPlugin::isModifierEnabled()
{
    return modifier_.isPreviewEnabled();
}

void HeightMapPlugin::applyModifier(Page *page)
{
    modifier_.applyModifier(page);
}
