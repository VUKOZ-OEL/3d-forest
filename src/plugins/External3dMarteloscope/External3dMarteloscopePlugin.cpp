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

/** @file External3dMarteloscopePlugin.cpp */

// Include 3D Forest.
#include <External3dMarteloscopePlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QDesktopServices>
#include <QProcess>
#include <QTcpSocket>
#include <QThread>
#include <QUrl>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopePlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

External3dMarteloscopePlugin::External3dMarteloscopePlugin()
    : mainWindow_(nullptr)
{
}

void External3dMarteloscopePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "External",
                              "External",
                              tr("3d-Marteloscope"),
                              tr("Start 3d-Marteloscope"),
                              ICON("external-3d-marteloscope"),
                              this,
                              SLOT(slotPlugin()),
                              MAIN_WINDOW_MENU_EXTERNAL_PRIORITY);
}

void External3dMarteloscopePlugin::slotPlugin()
{
    int port = 8501;

    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", port);
    bool connected = socket.waitForConnected(200);
    socket.abort();

    if (connected)
    {
        qDebug() << "Streamlit already running on port" << port
                 << ", reusing it.";
        QDesktopServices::openUrl(
            QUrl(QString("http://localhost:%1").arg(port)));
        return;
    }

    QString program = "python";

    QString app = QCoreApplication::applicationDirPath() +
                  "/plugins/3DForestExternal3dMarteloscopePlugin/python/app.py";

    QStringList arguments;
    arguments << "-m" << "streamlit" << "run" << app << "--server.port"
              << QString::number(port) << "--server.headless" << "true";

    qint64 pid;
    bool ok = QProcess::startDetached(program, arguments, QString(), &pid);
    if (!ok)
    {
        qWarning() << "Failed to start process";
        return;
    }

    qDebug() << "Started process with PID:" << pid;

    QThread::sleep(1);

    QDesktopServices::openUrl(QUrl(QString("http://localhost:%1").arg(port)));
}
