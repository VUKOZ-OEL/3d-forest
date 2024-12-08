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

/** @file HelpPlugin.cpp */

// Include 3D Forest.
#include <HelpPlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QMessageBox>

// Include local.
#define LOG_MODULE_NAME "HelpPlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/HelpResources/", name))

HelpPlugin::HelpPlugin() : mainWindow_(nullptr)
{
}

void HelpPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&aboutAction_,
                              "Help",
                              "",
                              tr("About 3D Forest"),
                              tr("About this application"),
                              THEME_ICON("question"),
                              this,
                              SLOT(slotAbout()),
                              MAIN_WINDOW_MENU_HELP_PRIORITY);
}

void HelpPlugin::slotAbout()
{
    QMessageBox::about(
        mainWindow_,
        tr("About 3D Forest, version ") + MainWindow::APPLICATION_VERSION,
        tr("3D Forest is software for analysis, processing, and visualization"
           " of Lidar point clouds, mainly focused on forest environment.\n"
           "\n"
           "Copyright 2020-present VUKOZ\n"
           "Blue Cat team and other authors\n"
           "https://www.3dforest.eu\n"
           "\n"
           "Uses delaunator-cpp, modified eigen, modified libigl, stb\n"
           "Uses modified octree, (c) University of Bonn\n"
           "Uses modified ctkRangeSlider, (c) Kitware Inc.\n"
           "Uses parts of modified Point Cloud Library in pcdio.\n"
           "\n"
           "3D Forest software license:\n"
           "This program is free software and is distributed under the GNU\n"
           "General Public License, version 3. In short, this means you are\n"
           "free to use 3D Forest for any purpose,"
           " commercial or non-commercial\n"
           "without any restrictions. You are also free to modify the program"
           "\nas you wish. If you distribute software which uses any part of\n"
           "3D Forest, you must provide access to the software's source code."
           "\nFor more details about the license, read the file LICENSE inside"
           "\nthe 3D Forest installation or source folder or see\n"
           "https://www.gnu.org/licenses/gpl-3.0.txt"));
}
