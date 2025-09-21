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

/** @file iland.cpp */

// Include std.
#include <iostream>

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Error.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QString>

// Include 3rd party.
#include "global.h"
#include "model.h"
#include "modelcontroller.h"
#include <version.h>

// Include local.
#define LOG_MODULE_NAME "iland"
#include <Log.hpp>

static std::string toStdString(const QString &str)
{
    return str.toUtf8().constData();
}

static void iLandModelRun(const QString &xmlName, int years)
{
    ModelController iLandModel;
    GlobalSettings::instance()->setModelController(&iLandModel);

    iLandModel.setFileName(xmlName);
    if (iLandModel.hasError())
    {
        THROW("setFileName: " + toStdString(iLandModel.lastError()));
    }

    iLandModel.create();
    if (iLandModel.hasError())
    {
        THROW("create: " + toStdString(iLandModel.lastError()));
    }

    iLandModel.run(years);
    if (iLandModel.hasError())
    {
        THROW("run: " + toStdString(iLandModel.lastError()));
    }
}

int main(int argc, char *argv[])
{
    int rc = 1;

    LOGGER_START_FILE("log_iland.txt");

    try
    {
        QCoreApplication app(argc, argv);

        ArgumentParser arg("executes iland-model");

        arg.add("-f",
                "--file",
                "",
                "Path to the input file to be processed.",
                true);

        arg.add("-y",
                "--years",
                "1",
                "The number of years to run the simulation");

        if (arg.parse(argc, argv))
        {
            QString xmlName = QString::fromStdString(arg.toString("--file"));
            int years = arg.toInt("--years");

            iLandModelRun(xmlName, years);
        }

        rc = 0;
    }
    catch (const IException &e)
    {
        std::cerr << "error: " << toStdString(e.message()) << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "error: unknown" << std::endl;
    }

    LOGGER_STOP_FILE;

    return rc;
}
