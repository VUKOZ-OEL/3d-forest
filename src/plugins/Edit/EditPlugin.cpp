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
#include <Application.hpp>
#include <EditPlugin.hpp>
#include <EditPluginResetElevation.hpp>
#include <EditPluginResetManagementStatus.hpp>
#include <EditPluginSetClassification.hpp>

// Include local.
#define LOG_MODULE_NAME "EditPlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

EditPlugin::EditPlugin() : app_(nullptr)
{
}

void EditPlugin::initialize(Application *app)
{
    app_ = app;

    app_->createAction(
        nullptr,
        "Edit",
        "Edit",
        tr("Reset elevation"),
        tr("Reset elevation"),
        ThemeIcon(),
        [this]() { slotResetElevation(); },
        MAIN_WINDOW_MENU_EDIT_PRIORITY);

    app_->createAction(
        nullptr,
        "Edit",
        "Edit",
        tr("Reset management status"),
        tr("Reset management status"),
        ThemeIcon(),
        [this]() { slotResetManagementStatus(); },
        MAIN_WINDOW_MENU_EDIT_PRIORITY);

    app_->createAction(
        nullptr,
        "Edit",
        "Edit",
        tr("Set classification"),
        tr("Set classification"),
        ThemeIcon(),
        [this]() { slotSetClassification(); },
        MAIN_WINDOW_MENU_EDIT_PRIORITY);
}

void EditPlugin::slotResetElevation()
{
    EditPluginResetElevation::run(app_);
}

void EditPlugin::slotResetManagementStatus()
{
    EditPluginResetManagementStatus::run(app_);
}

void EditPlugin::slotSetClassification()
{
    EditPluginSetClassification::run(app_);
}
