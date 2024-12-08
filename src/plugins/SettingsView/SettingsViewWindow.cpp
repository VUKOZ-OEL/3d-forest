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

/** @file SettingsViewWindow.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <SettingsViewWidget.hpp>
#include <SettingsViewWindow.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "SettingsViewWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/SettingsViewResources/", name))

SettingsViewWindow::SettingsViewWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow)
{
    // Widget.
    SettingsViewWidget *widget = new SettingsViewWidget(mainWindow);

    // Dock.
    setWidget(widget);
    setFixedHeight(widget->sizeHint().height());
    setWindowTitle(tr("View Settings"));
    setWindowIcon(ICON("brush"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);
}