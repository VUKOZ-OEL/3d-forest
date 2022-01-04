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

/** @file Application.cpp */

#include <ApplicationLog.hpp>
#include <QApplication>
#include <QSurfaceFormat>
#include <WindowMain.hpp>

int main(int argc, char *argv[])
{
    ApplicationLog::install();

    QApplication app(argc, argv);

    app.setOrganizationName("VUKOZ v.v.i.");
    app.setApplicationName(WindowMain::APPLICATION_NAME);
    app.setApplicationVersion(WindowMain::APPLICATION_VERSION);
    app.setWindowIcon(QIcon(":/3dforest-256.png"));

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    WindowMain window;
    window.setWindowIcon(QIcon(":/3dforest-256.png"));
    window.show();

    return app.exec();
}
