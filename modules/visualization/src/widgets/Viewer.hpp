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
    @file Viewer.hpp
*/

#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <Camera.hpp>
#include <QWidget>
#include <vector>

class Editor;
class GLWidget;

/** OpenGL Viewer. */
class Viewer : public QWidget
{
    Q_OBJECT

public:
    enum ViewLayout
    {
        VIEW_LAYOUT_SINGLE,
        VIEW_LAYOUT_TWO_COLUMNS
    };

    enum ViewCamera
    {
        VIEW_CAMERA_ORTHOGRAPHIC,
        VIEW_CAMERA_PERSPECTIVE,
        VIEW_CAMERA_TOP,
        VIEW_CAMERA_FRONT,
        VIEW_CAMERA_LEFT,
        VIEW_CAMERA_3D,
        VIEW_CAMERA_RESET_DISTANCE,
        VIEW_CAMERA_RESET_CENTER
    };

    explicit Viewer(QWidget *parent = nullptr);
    ~Viewer();

    void setLayout(ViewLayout viewLayout);
    void setViewport(ViewCamera viewCamera);

    void selectViewport(GLWidget *viewport);

    void updateScene(Editor *editor);
    void resetScene(Editor *editor);

    Camera camera() const;

signals:
    void cameraChanged(bool);

protected:
    std::vector<GLWidget *> viewports_;

    void initializeViewer();
    GLWidget *createViewport();
    GLWidget *selectedViewport();
    const GLWidget *selectedViewport() const;
};

#endif /* VIEWER_HPP */
