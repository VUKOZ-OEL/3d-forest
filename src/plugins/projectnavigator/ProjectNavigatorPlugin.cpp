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

#include <IconTheme.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorPlugin.hpp>
#include <ProjectNavigatorWindow.hpp>

#define ICON(name) (IconTheme(":/projectnavigator/", name))

ProjectNavigatorPlugin::ProjectNavigatorPlugin(MainWindow *mainWindow)
    : QObject(mainWindow),
      mainWindow_(mainWindow),
      projectWindow_(nullptr)
{
    projectWindow_ = new ProjectNavigatorWindow(mainWindow_);

    mainWindow_->createAction(nullptr,
                              "Tools",
                              "Tools",
                              tr("Project Navigator"),
                              tr("Show project navigator"),
                              ICON("blueprint"),
                              this,
                              SLOT(slotShowNavigator()));
}

void ProjectNavigatorPlugin::slotShowNavigator()
{
    projectWindow_->show();
    projectWindow_->raise();
    projectWindow_->activateWindow();
}