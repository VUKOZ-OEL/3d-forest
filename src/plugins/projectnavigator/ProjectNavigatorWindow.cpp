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

/** @file ProjectNavigatorWindow.cpp */

#include <IconTheme.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorClassifications.hpp>
#include <ProjectNavigatorFiles.hpp>
#include <ProjectNavigatorLayers.hpp>
#include <ProjectNavigatorWindow.hpp>

#include <QTabWidget>

ProjectNavigatorWindow::ProjectNavigatorWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tab
    datasets_ = new ProjectNavigatorFiles(mainWindow_);
    layers_ = new ProjectNavigatorLayers(mainWindow_);
    classifications_ = new ProjectNavigatorClassifications(mainWindow_);

    // Tabs
    tabWidget_ = new QTabWidget;
    tabWidget_->addTab(datasets_, tr("Files"));
    tabWidget_->addTab(layers_, tr("Layers"));
    tabWidget_->addTab(classifications_, tr("Class"));

    // Dock
    setWidget(tabWidget_);
    setWindowTitle(tr("Project Navigator"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
