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

/** @file MessageLogPlugin.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <MessageLogPlugin.hpp>
#include <MessageLogWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/MessageLogResources/", name))

MessageLogPlugin::MessageLogPlugin() : app_(nullptr), pluginWindow_(nullptr)
{
}

void MessageLogPlugin::initialize(Application *app)
{
    app_ = app;

    pluginWindow_ = new MessageLogWindow(app_);
    pluginWindow_->hide();
    if (globalLogThread)
    {
        globalLogThread->setCallback(pluginWindow_);
    }

    app_->createAction(
        nullptr,
        "File",
        "Windows",
        tr("Message Log"),
        tr("Show message log"),
        ICON("message-log"),
        [this]() { slotPlugin(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        90);

    // app_->hideToolBar("Windows");
}

void MessageLogPlugin::slotPlugin()
{
    if (pluginWindow_)
    {
        pluginWindow_->show();
        pluginWindow_->raise();
        pluginWindow_->activateWindow();
    }
}
