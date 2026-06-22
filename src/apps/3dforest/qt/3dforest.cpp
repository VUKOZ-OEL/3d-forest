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

/** @file 3dforest.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <QtApplication.hpp>

// Include local.
#define LOG_MODULE_NAME "3dforest"
#include <Log.hpp>
#include <WarningsDisable.hpp>

#ifndef GIT_COMMIT_HASH
    #define GIT_COMMIT_HASH "unknown"
#endif

static void messageLogWindowQtMessageHandler(QtMsgType type,
                                             const QMessageLogContext &context,
                                             const QString &msg);

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log.txt");
    // qInstallMessageHandler(messageLogWindowQtMessageHandler);

    LOG_INFO(<< "3DForest started. Git Revision <" << GIT_COMMIT_HASH << ">.");

    try
    {
        (void)qRegisterMetaType<size_t>("size_t");
        (void)qRegisterMetaType<LogMessage>("LogMessage");

        QtApplication app(argc, argv);

        app.setOrganizationName("VUKOZ v.v.i.");
        app.setApplicationName("3D Forest");
        app.setApplicationVersion("1.0");
        // app.setWindowIcon(QIcon(":/3d-forest-128px.png"));

        // MainWindow window;
        // window.setWindowIcon(QIcon(":/3d-forest-128px.png"));
        // window.show();

        app.init();

        rc = app.exec();
    }
    catch (std::exception &e)
    {
        LOG_ERROR("error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR("error: unknown");
    }

    LOGGER_STOP_FILE;

    return rc;
}

static void messageLogWindowQtMessageHandler(QtMsgType type,
                                             const QMessageLogContext &context,
                                             const QString &msg)
{
    (void)context;

    LogType logType;
    switch (type)
    {
        case QtDebugMsg:
            logType = LOG_TYPE_DEBUG;
            break;
        case QtWarningMsg:
            logType = LOG_TYPE_WARNING;
            break;
        case QtInfoMsg:
            logType = LOG_TYPE_INFO;
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
        default:
            logType = LOG_TYPE_ERROR;
            break;
    }

    LOG_MESSAGE(logType, "Qt", << msg.toStdString());
}

#include <WarningsEnable.hpp>
