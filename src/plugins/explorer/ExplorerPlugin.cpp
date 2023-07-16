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

/** @file ExplorerPlugin.cpp */

#include <ExplorerPlugin.hpp>
#include <ExplorerWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#define LOG_MODULE_NAME "ExplorerPlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/explorer/", name))

ExplorerPlugin::ExplorerPlugin() : mainWindow_(nullptr), projectWindow_(nullptr)
{
}

void ExplorerPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "File",
                              "File Properties",
                              tr("Explorer"),
                              tr("Show explorer"),
                              ICON("files"),
                              this,
                              SLOT(slotPlugin()));
}

void ExplorerPlugin::slotPlugin()
{
    if (!mainWindow_)
    {
        return;
    }

    if (!projectWindow_)
    {
        projectWindow_ = new ExplorerWindow(mainWindow_);
    }

    projectWindow_->show();
    projectWindow_->raise();
    projectWindow_->activateWindow();
}

QDockWidget *ExplorerPlugin::window() const
{
    return projectWindow_;
}
