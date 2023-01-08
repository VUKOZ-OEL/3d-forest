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

/** @file LoggerWindow.hpp */

#ifndef LOGGER_WINDOW_HPP
#define LOGGER_WINDOW_HPP

#include <ExportGui.hpp>
#include <Log.hpp>

#include <QDockWidget>

class MainWindow;

class QTextEdit;

/** Logger Window. */
class LoggerWindow : public QDockWidget, public LogThreadCallbackInterface
{
    Q_OBJECT

public:
    LoggerWindow(MainWindow *mainWindow);
    virtual ~LoggerWindow();

    virtual void println(const LogMessage &message);
    virtual void flush();

    static void EXPORT_GUI install();

public slots:
    void slotPrintln(const QString &time,
                     int type,
                     const QString &text,
                     const QString &module,
                     const QString &function);

signals:
    void signalPrintln(const QString &time,
                       int type,
                       const QString &text,
                       const QString &module,
                       const QString &function);

protected:
    MainWindow *mainWindow_;
    QTextEdit *textEdit_;
};

#endif /* LOGGER_WINDOW_HPP */
