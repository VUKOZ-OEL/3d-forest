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

/** @file External3dMarteloscopeRunner.hpp */

#ifndef EXTERNAL_3D_MARTELOSCOPE_RUNNER_HPP
#define EXTERNAL_3D_MARTELOSCOPE_RUNNER_HPP

// Include Std.

// Include 3D Forest.
class MainWindow;

// Include Qt.
#include <QObject>
#include <QPointer>
class QProcess;

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    class QWebEngineView;
#endif

/** External 3d-Marteloscope Runner. */
class External3dMarteloscopeRunner : public QObject
{
    Q_OBJECT

public:
    External3dMarteloscopeRunner(MainWindow *mainWindow);
    ~External3dMarteloscopeRunner();

    void start(const QString &pythonPath,
               const QString &appPath,
               const QString &projectPath,
               int startPort = 8501);

    void stop();
    bool isRunning() const;

private:
    bool isPortFree(int port) const;
    int findFreePort(int startPort) const;

    MainWindow *mainWindow_ = nullptr;

    QPointer<QProcess> process_;
    int port_ = -1;
    bool stopping_ = false;

#if defined(HAS_QT_WEB_ENGINE_WIDGETS)
    QPointer<QWebEngineView> view_;
#endif
};

#endif /* EXTERNAL_3D_MARTELOSCOPE_RUNNER_HPP */
