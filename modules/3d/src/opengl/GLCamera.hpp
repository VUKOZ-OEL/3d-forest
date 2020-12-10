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
    @file GLCamera.hpp
*/

#ifndef GL_CAMERA_HPP
#define GL_CAMERA_HPP

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QVector3D>
#include <QWheelEvent>
#include <vector>

/** OpenGL Camera. */
class GLCamera
{
public:
    GLCamera();
    ~GLCamera();

    // Viewport
    void setViewport(int x, int y, int width, int height);
    int width() const { return viewport_.width(); }
    int height() const { return viewport_.height(); }

    // Projection
    void setPerspective(float fovy, float zNear, float zFar);
    void setPerspective(float fovy, float aspect, float zNear, float zFar);

    // Model-view
    void setLookAt(const QVector3D &eye,
                   const QVector3D &center,
                   const QVector3D &up);

    void setDistance(float distance);
    float getDistance() const;

    const QVector3D &getEye() const { return eye_; }
    const QVector3D &getCenter() const { return center_; }
    const QVector3D &getUp() const { return up_; }

    QVector3D getDirection() const;
    QVector3D getView() const;
    QVector3D getRight() const;

    // Transform
    QVector3D project(const QVector3D &world) const;
    QVector3D unproject(const QVector3D &window) const;
    void getRay(int x, int y, QVector3D *base, QVector3D *direction);

    // Interaction
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void rotate(int dx, int dy);
    void pan(int dx, int dy);
    void zoom(int delta);

    // Matrix
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
    // Camera
    QVector3D eye_;
    QVector3D center_;
    QVector3D up_;

    // Viewport
    QRect viewport_;

    // Matrix
    QMatrix4x4 projection_;
    QMatrix4x4 projectionInv_;
    QMatrix4x4 modelView_;
    QMatrix4x4 modelViewInv_;
    QMatrix4x4 modelViewProjection_;
    QMatrix4x4 modelViewProjectionInv_;

    // Camera frustrum
    std::vector<float> frustrumPlanes_;

    // Interaction
    QPoint mouseLastPosition_;

    void setModelView(const QMatrix4x4 &m);
    void setProjection(const QMatrix4x4 &m);
    void setModelViewProjection(const QMatrix4x4 &m);
    void updateFrustrum();
    QVector3D rotate(const QVector3D &v, const QVector3D &axis, float angle);
};

#endif /* GL_CAMERA_HPP */
