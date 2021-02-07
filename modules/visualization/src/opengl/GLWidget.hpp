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
    @file GLWidget.hpp
*/

#ifndef GL_WIDGET_HPP
#define GL_WIDGET_HPP

#include <Editor.hpp>
#include <GLAabb.hpp>
#include <GLCamera.hpp>
#include <GLNode.hpp>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <Scene.hpp>
#include <vector>

class GLViewer;
class QMouseEvent;
class QWheelEvent;

/** OpenGL Widget. */
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    void setViewer(GLViewer *viewer);

    void setSelected(bool selected);
    bool isSelected() const;

    void updateScene(const Scene &scene);
    void updateScene(Editor *editor);

    void resetCamera();

protected:
    // Qt
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Specialized
    GLViewer *viewer_;
    bool selected_;

    std::vector<std::shared_ptr<GLNode>> nodes_;
    GLAabb aabb_;

    GLCamera camera_;

    void initializeGLWidget();
    void validateNodes();
};

#endif /* GL_WIDGET_HPP */
