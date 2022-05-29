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

/** @file SettingsPlugin.cpp */

#include <IconTheme.hpp>
#include <MainWindow.hpp>
#include <SettingsPlugin.hpp>
#include <SettingsWindow.hpp>

#define ICON(name) (IconTheme(":/settings/", name))

SettingsPlugin::SettingsPlugin(MainWindow *mainWindow)
    : QObject(mainWindow),
      mainWindow_(mainWindow),
      settingsWindow_(nullptr)
{
    settingsWindow_ = new SettingsWindow(mainWindow_);

    mainWindow_->createAction(nullptr,
                              "File",
                              "File Properties",
                              tr("Settings"),
                              tr("Show settings"),
                              ICON("settings"),
                              this,
                              SLOT(slotShowSettings()));
}

void SettingsPlugin::slotShowSettings()
{
    settingsWindow_->show();
    settingsWindow_->raise();
    settingsWindow_->activateWindow();
}
