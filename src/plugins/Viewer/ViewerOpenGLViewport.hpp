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

/** @file ViewerOpenGLViewport.hpp */

#ifndef VIEWER_OPENGL_VIEWPORT_HPP
#define VIEWER_OPENGL_VIEWPORT_HPP

// Include 3D Forest.
#include <Camera.hpp>
#include <Segment.hpp>
#include <ViewerAabb.hpp>
#include <ViewerCamera.hpp>
class Editor;
class ViewerViewports;
class ViewerOpenGLManager;

// Include Qt.
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
class QMouseEvent;
class QWheelEvent;

/** Viewer OpenGL Viewport. */
class ViewerOpenGLViewport : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ViewerOpenGLViewport(QWidget *parent = nullptr);
    ~ViewerOpenGLViewport();

    void setManager(ViewerOpenGLManager *manager);

    void setViewports(ViewerViewports *viewer, size_t viewportId);
    size_t viewportId() const;

    void setSelected(bool selected);
    bool selected() const;

    void updateScene(Editor *editor);
    void resetScene(Editor *editor, bool resetView);

    Camera camera() const;

    void setViewOrthographic();
    void setViewPerspective();
    void setViewTop();
    void setViewFront();
    void setViewRight();
    void setView3d();
    void setViewResetDistance();
    void setViewResetCenter();

protected:
    // Qt.
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    ViewerOpenGLManager *manager_;

    // Window Viewports.
    ViewerViewports *windowViewports_;
    size_t viewportId_;
    bool selected_;
    bool resized_;

    // Data.
    Editor *editor_;
    ViewerAabb aabb_;
    ViewerCamera camera_;

    void setViewDefault();
    void clearScreen();
    void renderScene();
    void renderFirstFrame();
    void renderLastFrame();
    void renderSegments();
    void renderAttributes();
    void renderLabels();
    bool skipSegmentRendering(const Segment &segment);
    void renderGuides();
    void renderSceneSettingsEnable();
    void renderSceneSettingsDisable();
    void cameraChanged();
    void setFocus();
    void setViewDirection(const QVector3D &dir, const QVector3D &up);

    class Object
    {
    public:
        size_t id{0};
        ViewerAabb aabb;
    };

    std::vector<Object> objects_;
    size_t selectedId{0};

    void updateObjects();
    void pickObject(const QPoint &p);
};

#endif /* VIEWER_OPENGL_VIEWPORT_HPP */
