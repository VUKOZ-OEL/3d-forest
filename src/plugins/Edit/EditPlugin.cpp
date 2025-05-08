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

/** @file EditPlugin.cpp */

// Include 3D Forest.
#include <EditPlugin.hpp>
#include <EditPluginResetElevation.hpp>
#include <EditPluginSetClassification.hpp>
#include <EditPluginSetManagementStatus.hpp>
#include <EditPluginSetSpecies.hpp>
#include <MainWindow.hpp>

// Include local.
#define LOG_MODULE_NAME "EditPlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

EditPlugin::EditPlugin() : mainWindow_(nullptr)
{
}

void EditPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Edit",
                              "Edit",
                              tr("Reset elevation"),
                              tr("Reset elevation"),
                              QIcon(),
                              this,
                              SLOT(slotResetElevation()),
                              MAIN_WINDOW_MENU_EDIT_PRIORITY);

    mainWindow_->createAction(nullptr,
                              "Edit",
                              "Edit",
                              tr("Set classification"),
                              tr("Set classification"),
                              QIcon(),
                              this,
                              SLOT(slotSetClassification()),
                              MAIN_WINDOW_MENU_EDIT_PRIORITY);

    mainWindow_->createAction(nullptr,
                              "Edit",
                              "Edit",
                              tr("Set management status"),
                              tr("Set management status"),
                              QIcon(),
                              this,
                              SLOT(slotSetManagementStatus()),
                              MAIN_WINDOW_MENU_EDIT_PRIORITY);

    mainWindow_->createAction(nullptr,
                              "Edit",
                              "Edit",
                              tr("Set species"),
                              tr("Set species"),
                              QIcon(),
                              this,
                              SLOT(slotSetSpecies()),
                              MAIN_WINDOW_MENU_EDIT_PRIORITY);
}

void EditPlugin::slotResetElevation()
{
    EditPluginResetElevation::run(mainWindow_);
}

void EditPlugin::slotSetClassification()
{
    EditPluginSetClassification::run(mainWindow_);
}

void EditPlugin::slotSetManagementStatus()
{
    EditPluginSetManagementStatus::run(mainWindow_);
}

void EditPlugin::slotSetSpecies()
{
    EditPluginSetSpecies::run(mainWindow_);
}
