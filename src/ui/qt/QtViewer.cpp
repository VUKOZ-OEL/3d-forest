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

/** @file QtViewer.cpp */

// Include 3D Forest.
#include <QtViewer.hpp>

// Include Qt.

// Include local.
#define LOG_MODULE_NAME "QtViewer"
#include <Log.hpp>

QtViewer::QtViewer(Viewer *viewer, QWidget *parent)
    : QOpenGLWidget(parent),
      viewer_(viewer)
{
    viewer_->updateRequested.connect([this]() { update(); });
}

QtViewer::~QtViewer()
{
}

void QtViewer::initializeGL()
{
    // Initialize OpenGL resources.
}

void QtViewer::resizeGL(int width, int height)
{
    // Update viewport/projection.
}

void QtViewer::paintGL()
{
    // Read scene/camera information from viewer_ and render it.
}
