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

/** @file GuiViewportOpenGL.hpp */

#ifndef GUI_VIEWPORT_OPENGL_HPP
#define GUI_VIEWPORT_OPENGL_HPP

#include <EditorCamera.hpp>

#include <GuiAabb.hpp>
#include <GuiCamera.hpp>

#include <QOpenGLFunctions>
#include <QOpenGLWidget>

class Editor;
class GuiViewports;
class QMouseEvent;
class QWheelEvent;

/** Gui Viewport OpenGL. */
class GuiViewportOpenGL : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GuiViewportOpenGL(QWidget *parent = nullptr);
    ~GuiViewportOpenGL();

    void setWindowViewports(GuiViewports *viewer, size_t viewportId);
    size_t viewportId() const;

    void setSelected(bool selected);
    bool isSelected() const;

    void updateScene(Editor *editor);
    void resetScene(Editor *editor, bool resetView);

    EditorCamera camera() const;

    void setViewOrthographic();
    void setViewPerspective();
    void setViewTop();
    void setViewFront();
    void setViewRight();
    void setView3d();
    void setViewResetDistance();
    void setViewResetCenter();

protected:
    // Qt
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Window Viewports
    GuiViewports *windowViewports_;
    size_t viewportId_;
    bool selected_;

    // Data
    Editor *editor_;
    GuiAabb aabb_;
    GuiCamera camera_;

    void resetCamera();
    void clearScreen();
    bool renderScene();
    void renderGuides();
    void renderSceneSettingsEnable();
    void renderSceneSettingsDisable();
    void cameraChanged();
    void setFocus();
    void setViewDirection(const QVector3D &dir, const QVector3D &up);
};

#endif /* GUI_VIEWPORT_OPENGL_HPP */
