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

/** @file SettingsWindow.cpp */

#include <MainWindow.hpp>
#include <SettingsViewWidget.hpp>
#include <SettingsWindow.hpp>

#include <QTabWidget>

SettingsWindow::SettingsWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Tabs
    viewWidget_ = new SettingsViewWidget(mainWindow_);

    // Tab
    tabWidget_ = new QTabWidget;
    tabWidget_->addTab(viewWidget_, tr("View"));
    tabWidget_->setFixedHeight(330);

    // Dock
    setWidget(tabWidget_);
    setWindowTitle(tr("Settings"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::LeftDockWidgetArea, this);
}
