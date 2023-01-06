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

/** @file LoggerPlugin.cpp */

#include <LoggerPlugin.hpp>
#include <LoggerWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/logger/", name))

LoggerPlugin::LoggerPlugin() : mainWindow_(nullptr), loggerWindow_(nullptr)
{
}

void LoggerPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    loggerWindow_ = new LoggerWindow(mainWindow_);
    loggerWindow_->hide();
    if (globalLogThread)
    {
        globalLogThread->setCallback(loggerWindow_);
    }

    mainWindow_->createAction(nullptr,
                              "File",
                              "File Properties",
                              tr("Logger"),
                              tr("Show Logger Window"),
                              ICON("logger"),
                              this,
                              SLOT(slotPlugin()));
}

void LoggerPlugin::slotPlugin()
{
    if (loggerWindow_)
    {
        loggerWindow_->show();
        loggerWindow_->raise();
        loggerWindow_->activateWindow();
    }
}
