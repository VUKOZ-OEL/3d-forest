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

#include <HelpPlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QMessageBox>

#define ICON(name) (ThemeIcon(":/help/", name))

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
                              SLOT(slotAbout()));
}

void HelpPlugin::slotAbout()
{
    QMessageBox::about(
        mainWindow_,
        tr("About 3D Forest, version ") + MainWindow::APPLICATION_VERSION,
        tr("3D Forest is software for analysis of Lidar data from forest"
           " environment.\n"
           "\n"
           "Copyright 2020-present VUKOZ\n"
           "Blue Cat team and other authors\n"
           "https://www.3dforest.eu\n"
           "\n"
           "Uses delaunator-cpp, modified eigen, modified libigl\n"
           "Uses modified ctkRangeSlider, (c) Kitware Inc.\n"
           "\n"
           "3D Forest software license:\n"
           "This program is free software and is distributed under the GNU\n"
           "General Public License, version 3. In short, this means you are\n"
           "free to use and distribute 3D Forest for any purpose, commercial\n"
           "or non-commercial, without any restrictions. You are also free to\n"
           "modify the program as you wish, with the only restriction that if\n"
           "you distribute the modified version, you must provide access to\n"
           "its source code.\n"
           "For more details about the license, read the file LICENSE inside\n"
           "the 3D Forest installation folder or see\n"
           "https://www.gnu.org/licenses/gpl-3.0.txt"));
}
