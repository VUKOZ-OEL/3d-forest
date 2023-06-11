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
#include <ProjectNavigatorItemColor.hpp>
#include <ProjectNavigatorItemCustomColor.hpp>
#include <ProjectNavigatorItemDescriptor.hpp>
#include <ProjectNavigatorItemElevation.hpp>
#include <ProjectNavigatorItemFiles.hpp>
#include <ProjectNavigatorItemIntensity.hpp>
#include <ProjectNavigatorItemLayers.hpp>
#include <ProjectNavigatorItemReturnNumber.hpp>
#include <ProjectNavigatorTree.hpp>
#include <ProjectNavigatorWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorWindow::ProjectNavigatorWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tab
    items_.push_back(
        new ProjectNavigatorItemFiles(mainWindow_, ICON("file"), tr("Files")));

    items_.push_back(new ProjectNavigatorItemLayers(mainWindow_,
                                                    ICON("layers"),
                                                    tr("Layers")));

    items_.push_back(
        new ProjectNavigatorItemClassifications(mainWindow_,
                                                ICON("classification"),
                                                tr("Classifications")));

    items_.push_back(
        new ProjectNavigatorItemColor(mainWindow_, ICON("color"), tr("Color")));

    items_.push_back(new ProjectNavigatorItemCustomColor(mainWindow_,
                                                         ICON("color"),
                                                         tr("Custom Color")));

    items_.push_back(new ProjectNavigatorItemIntensity(mainWindow_,
                                                       ICON("intensity"),
                                                       tr("Intensity")));

    items_.push_back(new ProjectNavigatorItemReturnNumber(mainWindow_,
                                                          ICON("return_number"),
                                                          tr("Return number")));

    items_.push_back(new ProjectNavigatorItemElevation(mainWindow_,
                                                       ICON("elevation_filter"),
                                                       tr("Elevation")));

    items_.push_back(
        new ProjectNavigatorItemDescriptor(mainWindow_,
                                           ICON("descriptor_filter"),
                                           tr("Descriptor")));

    items_.push_back(new ProjectNavigatorItemClipping(mainWindow_,
                                                      ICON("clip_filter"),
                                                      tr("Clip filter")));

    // Tabs
    menu_ = new ProjectNavigatorTree(mainWindow_);
    for (size_t i = 0; i < items_.size(); i++)
    {
        menu_->addItem(items_[i]);
    }

    // Dock
    setWidget(menu_);
    setWindowTitle(tr("Project Explorer"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
