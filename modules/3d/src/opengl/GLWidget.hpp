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

#include <GLAabb.hpp>
#include <GLCamera.hpp>
#include <GLNode.hpp>
#include <Node.hpp>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <vector>

class Viewer;

/** OpenGL Widget. */
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    void setViewer(Viewer *viewer);

    void setSelected(bool selected);
    bool isSelected() const;

    void updateScene(const std::vector<std::shared_ptr<Node>> &scene);

protected:
    // Qt
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Specialized
    Viewer *viewer_;
    bool selected_;

    std::vector<std::shared_ptr<GLNode>> nodes_;
    GLAabb aabb_;

    GLCamera camera_;

    void initializeGLWidget();
    void validateNodes();
};

#endif /* GL_WIDGET_HPP */
