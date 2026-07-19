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

/** @file MessageLogWindow.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <MessageLogWindow.hpp>
#include <TextEdit.hpp>

// #define MESSAGE_LOG_WINDOW_DEBUG_PRINT 1
#define MESSAGE_LOG_WINDOW_FILE_NAME "log.txt"
#define MESSAGE_LOG_WINDOW_FILE_SIZE_MAX (100 * 1024 * 1024)

MessageLogWindow::MessageLogWindow(Application *app)
    : DockWidget(app),
      app_(app)
{
    // Widget.
    textEdit_ = new TextEdit;
    textEdit_->setReadOnly(true);

    // File.
    file_.open(MESSAGE_LOG_WINDOW_FILE_NAME, "w+t");

    // Dock.
    setWidget(textEdit_);
    setWindowTitle(tr("Message Log"));
    setAllowedAreas(Ui::LeftDockWidgetArea | Ui::RightDockWidgetArea |
                    Ui::TopDockWidgetArea | Ui::BottomDockWidgetArea);
    app->addDockWidget(Ui::BottomDockWidgetArea, this);
}

MessageLogWindow::~MessageLogWindow()
{
    if (globalLogThread)
    {
        globalLogThread->setCallback(nullptr);
    }
}

void MessageLogWindow::println(const LogMessage &message)
{
    app_->post([this, message]() { slotPrintln(message); });
}

void MessageLogWindow::flush()
{
    // Empty.
}

void MessageLogWindow::slotPrintln(const LogMessage &message)
{
    if (file_.size() > MESSAGE_LOG_WINDOW_FILE_SIZE_MAX)
    {
        textEdit_->clear();
        file_.open(MESSAGE_LOG_WINDOW_FILE_NAME, "w+t");
    }

#if defined(MESSAGE_LOG_WINDOW_DEBUG_PRINT)
    std::string line = toString(message.threadId) + " " +
                   message.time +
                   LogMessage::typeString(message.type) +
                   message.text) + " [" +
                   message.module + ":" +
                   message.function + "] " +
                   toString(file_.size());

    if (threadId_ != 0 && threadId_ != message.threadId)
    {
        file_.write("\n");
    }
    threadId_ = message.threadId;
#else
    std::string line = message.time + LogMessage::typeString(message.type) +
                       message.text + " [" + message.module + ":" +
                       message.function + "] " + toString(message.threadId);
#endif

    textEdit_->append(line);
    file_.write(line + "\n");
}
