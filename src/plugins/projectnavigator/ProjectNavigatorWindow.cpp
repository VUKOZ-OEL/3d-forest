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
#include <ProjectNavigatorItemClassifications.hpp>
#include <ProjectNavigatorItemClipping.hpp>
#include <ProjectNavigatorItemDescriptor.hpp>
#include <ProjectNavigatorItemElevation.hpp>
#include <ProjectNavigatorItemFiles.hpp>
#include <ProjectNavigatorItemLayers.hpp>
#include <ProjectNavigatorTree.hpp>
#include <ProjectNavigatorWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorWindow::ProjectNavigatorWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tab
    datasets_ = new ProjectNavigatorItemFiles(mainWindow_);
    layers_ = new ProjectNavigatorItemLayers(mainWindow_);
    classifications_ = new ProjectNavigatorItemClassifications(mainWindow_);
    clipping_ = new ProjectNavigatorItemClipping(mainWindow_);
    descriptor_ = new ProjectNavigatorItemDescriptor(mainWindow_);
    elevation_ = new ProjectNavigatorItemElevation(mainWindow_);

    // Tabs
    menu_ = new ProjectNavigatorTree(mainWindow_);
    menu_->addItem(datasets_, ICON("file"), tr("Files"));
    menu_->addItem(layers_, ICON("layers"), tr("Layers"));
    menu_->addItem(classifications_,
                   ICON("classification"),
                   tr("Classifications"));
    menu_->addItem(elevation_, ICON("elevation_filter"), tr("Elevation"));
    menu_->addItem(descriptor_, ICON("descriptor_filter"), tr("Descriptor"));
    menu_->addItem(clipping_, ICON("clip_filter"), tr("Clip filter"));

    // Dock
    setWidget(menu_);
    setWindowTitle(tr("Project Navigator"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
