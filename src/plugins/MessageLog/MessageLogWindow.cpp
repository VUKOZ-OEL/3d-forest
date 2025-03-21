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
#include <MainWindow.hpp>
#include <MessageLogWindow.hpp>

// Include Qt.
#include <QTextEdit>

// #define MESSAGE_LOG_WINDOW_DEBUG_PRINT 1
#define MESSAGE_LOG_WINDOW_FILE_NAME "log.txt"
#define MESSAGE_LOG_WINDOW_FILE_SIZE_MAX (100 * 1024 * 1024)

MessageLogWindow::MessageLogWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Widget.
    textEdit_ = new QTextEdit;
    textEdit_->setReadOnly(true);

    // File.
    file_.open(MESSAGE_LOG_WINDOW_FILE_NAME, "w+t");

    // Dock.
    setWidget(textEdit_);
    setWindowTitle(tr("Message Log"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                    Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, this);

    // Signals.
    connect(this,
            SIGNAL(signalPrintln(const LogMessage &)),
            this,
            SLOT(slotPrintln(const LogMessage &)),
            Qt::QueuedConnection);
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
    emit signalPrintln(message);
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
    QString line = QString::number(message.threadId) + " " +
                   QString::fromStdString(message.time) +
                   QString(LogMessage::typeString(message.type)) +
                   QString::fromStdString(message.text) + " [" +
                   QString::fromStdString(message.module) + ":" +
                   QString::fromStdString(message.function) + "] " +
                   QString::number(file_.size());

    if (threadId_ != 0 && threadId_ != message.threadId)
    {
        file_.write("\n");
    }
    threadId_ = message.threadId;
#else
    QString line = QString::fromStdString(message.time) +
                   QString(LogMessage::typeString(message.type)) +
                   QString::fromStdString(message.text) + " [" +
                   QString::fromStdString(message.module) + ":" +
                   QString::fromStdString(message.function) + "] " +
                   QString::number(message.threadId);
#endif

    textEdit_->append(line);
    file_.write(line.toStdString() + "\n");
}
