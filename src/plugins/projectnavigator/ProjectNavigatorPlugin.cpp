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

/** @file ProjectNavigatorPlugin.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorPlugin.hpp>
#include <ProjectNavigatorWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorPlugin::ProjectNavigatorPlugin()
    : mainWindow_(nullptr),
      projectWindow_(nullptr)
{
}

void ProjectNavigatorPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "File",
                              "File Properties",
                              tr("Project Navigator"),
                              tr("Show project navigator"),
                              ICON("project"),
                              this,
                              SLOT(slotPlugin()));
}

void ProjectNavigatorPlugin::slotPlugin()
{
    if (!mainWindow_)
    {
        return;
    }

    if (!projectWindow_)
    {
        projectWindow_ = new ProjectNavigatorWindow(mainWindow_);
    }

    projectWindow_->show();
    projectWindow_->raise();
    projectWindow_->activateWindow();
}
