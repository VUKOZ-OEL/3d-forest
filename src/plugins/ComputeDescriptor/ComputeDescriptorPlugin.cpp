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

/** @file ComputeDescriptorPlugin.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ComputeDescriptorPlugin.hpp>
#include <ComputeDescriptorWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeDescriptorPlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeDescriptorResources/", name))

void ComputeDescriptorPlugin::initialize(Application *app)
{
    app_ = app;

    app_->createAction(this,
                       {{"Compute", MAIN_WINDOW_MENU_COMPUTE_PRIORITY}},
                       "Compute",
                       tr("Descriptor"),
                       tr("Compute descriptor of each point"),
                       ICON("descriptor"),
                       {},
                       new ComputeDescriptorWidget(app_));
}
