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

/**
    @file Forest3dApplication.cpp
*/

#include <Forest3dWindow.hpp>
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("VUKOZ v.v.i.");
    app.setApplicationName(Forest3dWindow::APPLICATION_NAME);
    app.setApplicationVersion(Forest3dWindow::APPLICATION_VERSION);
    app.setWindowIcon(QIcon(":/3dforest-256.png"));

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    Forest3dWindow mainWindow;
    mainWindow.setWindowIcon(QIcon(":/3dforest-256.png"));
    mainWindow.show();

    return app.exec();
}
