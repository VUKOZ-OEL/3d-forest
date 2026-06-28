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

/** @file QtApplication.cpp */

// Include std.

// Include 3D Forest.
#include <QtApplication.hpp>

// Include Qt.
// #include <QSurfaceFormat>

// Include local.
#define LOG_MODULE_NAME "QtApplication"
#include <Log.hpp>

QtApplication::QtApplication(int &argc, char **argv) : qapplication_(argc, argv)
{
}

QtApplication::~QtApplication()
{
}

void QtApplication::init()
{
    // QSurfaceFormat format;
    // format.setDepthBufferSize(24);
    // format.setAlphaBufferSize(8);
    // format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    // QSurfaceFormat::setDefaultFormat(format);

    load();
}

void QtApplication::setOrganizationName(const std::string &str)
{
}

void QtApplication::setApplicationName(const std::string &str)
{
}

void QtApplication::setApplicationVersion(const std::string &str)
{
}

int QtApplication::exec()
{
    return 0;
}
