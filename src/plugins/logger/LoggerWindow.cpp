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

/** @file LoggerWindow.cpp */

#include <Log.hpp>
#include <LoggerWindow.hpp>
#include <MainWindow.hpp>

#include <QTextEdit>

LoggerWindow::LoggerWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // widget
    textEdit_ = new QTextEdit;
    textEdit_->setReadOnly(true);

    // Dock
    setWidget(textEdit_);
    setWindowTitle(tr("Logger"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                    Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, this);

    // signals
    connect(this,
            SIGNAL(signalPrintln(const QString &, const QString &)),
            this,
            SLOT(slotPrintln(const QString &, const QString &)),
            Qt::QueuedConnection);
}

LoggerWindow::~LoggerWindow()
{
    if (globalLogThread)
    {
        globalLogThread->setCallback(nullptr);
    }
}

void LoggerWindow::println(const LogMessage &message)
{
    emit signalPrintln(QString::fromStdString(message.time),
                       QString::fromStdString(message.text));
}

void LoggerWindow::flush()
{
    // empty
}

void LoggerWindow::slotPrintln(const QString &time, const QString &text)
{
    textEdit_->append(time + " " + text);
}

static void loggerWindowQtMessageHandler(QtMsgType type,
                                         const QMessageLogContext &context,
                                         const QString &msg)
{
    (void)context;

    LogType logType;
    switch (type)
    {
        case QtDebugMsg:
            logType = LOG_DEBUG;
            break;
        case QtWarningMsg:
            logType = LOG_WARNING;
            break;
        case QtInfoMsg:
            logType = LOG_INFO;
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
        default:
            logType = LOG_ERROR;
            break;
    }

    LOG_MESSAGE(logType, "Qt", << msg.toStdString());
}

void LoggerWindow::install()
{
    qInstallMessageHandler(loggerWindowQtMessageHandler);
}
