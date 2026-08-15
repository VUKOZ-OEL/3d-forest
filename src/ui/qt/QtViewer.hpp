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

/** @file QtViewer.hpp */

#ifndef QT_VIEWER_HPP
#define QT_VIEWER_HPP

// Include 3D Forest.
#include <Viewer.hpp>

// Include Qt.
#include <QOpenGLWidget>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtViewer. */
class EXPORT_UI_QT QtViewer : public QOpenGLWidget
{
public:
    explicit QtViewer(Viewer *viewer, QWidget *parent = nullptr);
    virtual ~QtViewer();

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
    Viewer *viewer_;
};

#include <WarningsEnable.hpp>

#endif /* QT_VIEWER_HPP */
