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

/** @file ExplorerWindow.cpp */

// Include 3D Forest.
#include <ExplorerClassificationsWidget.hpp>
#include <ExplorerClippingWidget.hpp>
#include <ExplorerDescriptorWidget.hpp>
#include <ExplorerElevationWidget.hpp>
#include <ExplorerFilesWidget.hpp>
#include <ExplorerIntensityWidget.hpp>
#include <ExplorerSegmentsWidget.hpp>
#include <ExplorerWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <ToolTabWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "ExplorerWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/explorer/", name))

ExplorerWindow::ExplorerWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tab.
    items_.push_back(
        new ExplorerFilesWidget(mainWindow_, ICON("file"), tr("Files")));

    items_.push_back(new ExplorerSegmentsWidget(mainWindow_,
                                                ICON("segments"),
                                                tr("Segments")));

    items_.push_back(new ExplorerClassificationsWidget(mainWindow_,
                                                       ICON("classification"),
                                                       tr("Classifications")));

    items_.push_back(new ExplorerIntensityWidget(mainWindow_,
                                                 ICON("intensity"),
                                                 tr("Intensity")));

    items_.push_back(new ExplorerElevationWidget(mainWindow_,
                                                 ICON("elevation_filter"),
                                                 tr("Elevation")));

    items_.push_back(new ExplorerDescriptorWidget(mainWindow_,
                                                  ICON("descriptor_filter"),
                                                  tr("Descriptor")));

    items_.push_back(new ExplorerClippingWidget(mainWindow_,
                                                ICON("clip_filter"),
                                                tr("Clip filter")));

    // Tabs.
    tabWidget_ = new ToolTabWidget;
    for (size_t i = 0; i < items_.size(); i++)
    {
        tabWidget_->addTab(items_[i], items_[i]->icon(), items_[i]->text());
    }

    // Dock.
    setWidget(tabWidget_);
    setWindowTitle(tr("Explorer"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}
