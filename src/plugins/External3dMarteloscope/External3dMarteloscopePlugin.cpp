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
#include <External3dMarteloscopeRunner.hpp>
#include <File.hpp>
#include <FileFormatCsv.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QDir>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopePlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

static std::string toStdString(const QString &str)
{
    return str.toUtf8().constData();
}

External3dMarteloscopePlugin::External3dMarteloscopePlugin()
    : mainWindow_(nullptr),
      runner_(nullptr)
{
}

External3dMarteloscopePlugin::~External3dMarteloscopePlugin()
{
    if (runner_ && runner_->isRunning())
    {
        runner_->stop();
    }
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
    std::string errorMessage;

    if (!runner_)
    {
        runner_ = new External3dMarteloscopeRunner(mainWindow_);
    }

    try
    {
        run();
    }
    catch (std::exception &e)
    {
        errorMessage = e.what();
    }
    catch (...)
    {
        errorMessage = "unknown";
    }

    if (!errorMessage.empty())
    {
        mainWindow_->showError(errorMessage.c_str());
    }
}

void External3dMarteloscopePlugin::run()
{
    std::string projectPath = mainWindow_->editor().projectPath();

    runPythonApp(projectPath);
}

void External3dMarteloscopePlugin::runPythonApp(const std::string &projectPath)
{
    LOG_DEBUG(<< "Start python app with project <" << projectPath << ">.");

    QString appDir = QCoreApplication::applicationDirPath();

    QString pythonHome;
    QString pythonPath;
    QString pythonExe = QDir(appDir).filePath("python/python.exe");
    QString pyhonScript = QDir(appDir).filePath(
                "plugins/3DForestExternal3dMarteloscopePlugin/python/app.py");

    if (!QFile::exists(pythonExe))
    {
        // Use system Python if bundled version is not found.
        // This is intended for use in a development environment.
        pythonExe = "python";
    }
    else
    {
        // Use bundled Python
        pythonHome = QDir(appDir).filePath("python");
        pythonPath = QDir(appDir).filePath("python/Lib");
    }

    runner_->start(pythonHome,
                   pythonPath,
                   pythonExe,
                   pyhonScript,
                   QString::fromStdString(projectPath));

    LOG_DEBUG(<< "Finished starting python app.");
}
