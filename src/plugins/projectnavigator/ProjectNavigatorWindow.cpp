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

#include <MainWindow.hpp>
#include <ProjectNavigatorClassifications.hpp>
#include <ProjectNavigatorClipping.hpp>
#include <ProjectNavigatorElevation.hpp>
#include <ProjectNavigatorFiles.hpp>
#include <ProjectNavigatorLayers.hpp>
#include <ProjectNavigatorWindow.hpp>
#include <ThemeIcon.hpp>
#include <ToolTabWidget.hpp>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorWindow::ProjectNavigatorWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tab
    datasets_ = new ProjectNavigatorFiles(mainWindow_);
    layers_ = new ProjectNavigatorLayers(mainWindow_);
    classifications_ = new ProjectNavigatorClassifications(mainWindow_);
    elevation_ = new ProjectNavigatorElevation(mainWindow_);
    clipping_ = new ProjectNavigatorClipping(mainWindow_);

    // Tabs
    tabWidget_ = new ToolTabWidget;
    tabWidget_->addTab(datasets_, ICON("file"), tr("File"));
    tabWidget_->addTab(layers_, ICON("layers"), tr("Layer"));
    tabWidget_->addTab(classifications_, ICON("classification"), tr("Class"));
    tabWidget_->addTab(elevation_, ICON("elevation_filter"), tr("Elevation"));
    tabWidget_->addTab(clipping_, ICON("clip_filter"), tr("Clip"));

    // Dock
    setWidget(tabWidget_);
    setWindowTitle(tr("Project Navigator"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
