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
#include <MainWindow.hpp>
#include <MessageLogPlugin.hpp>
#include <MessageLogWindow.hpp>
#include <ThemeIcon.hpp>

#define ICON(name) (ThemeIcon(":/messagelog/", name))

MessageLogPlugin::MessageLogPlugin()
    : mainWindow_(nullptr),
      messageLogWindow_(nullptr)
{
}

void MessageLogPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    messageLogWindow_ = new MessageLogWindow(mainWindow_);
    messageLogWindow_->hide();
    if (globalLogThread)
    {
        globalLogThread->setCallback(messageLogWindow_);
    }

    mainWindow_->createAction(nullptr,
                              "File",
                              "Windows",
                              tr("Message Log"),
                              tr("Show message log"),
                              ICON("message_log"),
                              this,
                              SLOT(slotPlugin()));

    mainWindow_->hideToolBar("Windows");
}

void MessageLogPlugin::slotPlugin()
{
    if (messageLogWindow_)
    {
        messageLogWindow_->show();
        messageLogWindow_->raise();
        messageLogWindow_->activateWindow();
    }
}
