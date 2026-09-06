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
#include <Camera.hpp>
#include <QtViewerAabb.hpp>
#include <QtViewerCamera.hpp>
#include <Segments.hpp>
#include <Viewer.hpp>
class Editor;
class QtApplication;
class QtViewerViewports;
class QtViewerOpenGLManager;

// Include Qt.
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
class QMouseEvent;
class QWheelEvent;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** Qt Viewer. */
class EXPORT_UI_QT QtViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    explicit QtViewer(Viewer *viewer,
                      QtApplication *app,
                      QWidget *parent = nullptr);
    virtual ~QtViewer();

    size_t viewportId() const;

    void setSelected(bool selected);
    bool selected() const;

    void updateScene();
    void resetScene();
    void resetSceneView();

    Camera camera() const;

    void setViewOrthographic();
    void setViewPerspective();
    void setView2d();

    void setViewTop();
    void setViewFront();
    void setViewRight();
    void setView3d();
    void setViewResetDistance();
    void setViewResetCenter();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QtApplication *app_;
    std::shared_ptr<QtViewerOpenGLManager> manager_;

    // Window Viewports.
    Viewer *viewer_;
    size_t viewportId_;
    bool selected_;
    bool resized_;

    // Data.
    Editor *editor_;
    QtViewerAabb aabb_;
    QtViewerCamera camera_;

    void setViewDefault();
    void clearScreen();

    void renderScene();
    void renderFirstFrame();
    void renderFirstFrameData();
    void renderLastFrame();

    void renderSegmentsSelection();
    void renderSegmentsDbh();
    void renderSegmentsHullProjections();
    void renderSegmentMeshes(const Segment &segment, bool onlyProjections);
    void renderSegmentsMeshes();
    void renderSegmentsAttributes();
    void renderSegmentsLabels();

    bool skipSegment(const Segment &segment);
    bool skipSegmentByFilter(const Segment &segment);
    bool skipSegmentByAttributes(const Segment &segment);

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
        QtViewerAabb aabb;
        QVector3D dbhPosition;
        float dbh{0.0F};
        bool selected{false};
    };

    std::vector<Object> objects_;
    Segments segments_;

    void updateObjects();
    void pickObject(const QPoint &p, bool ctrl);
    size_t pickObject2D(const QVector3D &p1, const QVector3D &p2);
    size_t pickObject3D(const QVector3D &p1, const QVector3D &p2);
};

#include <WarningsEnable.hpp>

#endif /* QT_VIEWER_HPP */
