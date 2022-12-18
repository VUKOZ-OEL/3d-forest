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
    classifications_ =
        new ProjectNavigatorItemClassifications(mainWindow_,
                                                ICON("classification"),
                                                tr("Classifications"));
    clipping_ = new ProjectNavigatorItemClipping(mainWindow_,
                                                 ICON("clip_filter"),
                                                 tr("Clip filter"));
    descriptor_ = new ProjectNavigatorItemDescriptor(mainWindow_,
                                                     ICON("descriptor_filter"),
                                                     tr("Descriptor"));
    elevation_ = new ProjectNavigatorItemElevation(mainWindow_,
                                                   ICON("elevation_filter"),
                                                   tr("Elevation"));
    files_ =
        new ProjectNavigatorItemFiles(mainWindow_, ICON("file"), tr("Files"));
    layers_ = new ProjectNavigatorItemLayers(mainWindow_,
                                             ICON("layers"),
                                             tr("Layers"));

    // Tabs
    menu_ = new ProjectNavigatorTree(mainWindow_);
    menu_->addItem(files_);
    menu_->addItem(layers_);
    menu_->addItem(classifications_);
    menu_->addItem(elevation_);
    menu_->addItem(descriptor_);
    menu_->addItem(clipping_);

    // Dock
    setWidget(menu_);
    setWindowTitle(tr("Project Navigator"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
