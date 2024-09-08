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

/** @file ViewerCamera.hpp */

#ifndef VIEWER_CAMERA_HPP
#define VIEWER_CAMERA_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Camera.hpp>

// Include Qt.
#include <QMatrix4x4>
#include <QPoint>
#include <QQuaternion>
#include <QRect>
#include <QVector3D>

class QMouseEvent;
class QWheelEvent;

/** Viewer Camera. */
class ViewerCamera
{
public:
    ViewerCamera();
    ~ViewerCamera();

    // Viewport.
    void setViewportId(size_t id);
    void setViewport(int x, int y, int width, int height);
    int width() const { return viewport_.width(); }
    int height() const { return viewport_.height(); }

    // Projection.
    void updateProjection();
    void setPerspective();
    void setOrthographic();
    void setPerspective(float fovy, float zNear, float zFar);
    void setPerspective(float fovy, float aspect, float zNear, float zFar);

    // Model-view.
    void setLookAt(const QVector3D &center, float distance);

    void setLookAt(const QVector3D &eye,
                   const QVector3D &center,
                   const QVector3D &up);

    void setDistance(float distance);

    // Camera.
    float getDistance() const { return distance_; }
    const QVector3D &getEye() const { return eye_; }
    const QVector3D &getCenter() const { return center_; }
    const QVector3D &getUp() const { return up_; }
    const QVector3D &getRight() const { return right_; }
    const QVector3D &getDirection() const { return direction_; }
    QMatrix4x4 rotation() const;

    Camera toCamera() const;

    // Transform.
    QVector3D project(const QVector3D &world) const;
    QVector3D unproject(const QVector3D &window) const;
    void getRay(int x, int y, QVector3D *base, QVector3D *direction);

    // Interaction.
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void rotate(int dx, int dy);
    void pan(int dx, int dy);
    void zoom(int dy);

    // Matrix.
    const QMatrix4x4 &getModelView() const { return modelView_; }
    const QMatrix4x4 &getProjection() const { return projection_; }
    const QMatrix4x4 &getModelViewProjection() const
    {
        return modelViewProjection_;
    }

    const QMatrix4x4 &getModelViewInv() const { return modelViewInv_; }
    const QMatrix4x4 &getProjectionInv() const { return projectionInv_; }
    const QMatrix4x4 &getModelViewProjectionInv() const
    {
        return modelViewProjectionInv_;
    }

protected:
    // Camera.
    QVector3D eye_;
    QVector3D center_;
    QVector3D right_;
    QVector3D up_;
    QVector3D direction_;
    QQuaternion rotation_;
    float distance_;
    float fov_;
    float zNear_;
    float zFar_;
    bool perspective_;

    // Viewport.
    size_t viewportId_;
    QRect viewport_;

    // Matrix.
    QMatrix4x4 projection_;
    QMatrix4x4 projectionInv_;
    QMatrix4x4 modelView_;
    QMatrix4x4 modelViewInv_;
    QMatrix4x4 modelViewProjection_;
    QMatrix4x4 modelViewProjectionInv_;

    // Camera frustum.
    std::vector<float> frustrumPlanes_;

    // Interaction.
    QPoint mouseLastPosition_;
    float sensitivityX_;
    float sensitivityY_;
    float sensitivityZoom_;

    void updateMatrix();
    void setModelView(const QMatrix4x4 &m);
    void setProjection(const QMatrix4x4 &m);
    void setModelViewProjection(const QMatrix4x4 &m);
    void updateFrustrum();
};

#endif /* VIEWER_CAMERA_HPP */
