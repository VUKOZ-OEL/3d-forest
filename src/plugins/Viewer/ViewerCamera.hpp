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
#include <ViewerUtil.hpp>

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

    void setLookAt(const QVector3D &dir,
                   float distance,
                   const QVector3D &center,
                   const QVector3D &up);

    void setDistance(float distance);

    // Camera.
    float distance() const { return distance_; }
    const QVector3D &eye() const { return eye_; }
    const QVector3D &center() const { return center_; }
    const QVector3D &offset() const { return offset_; }
    const QVector3D &up() const { return up_; }
    const QVector3D &right() const { return right_; }
    const QVector3D &direction() const { return direction_; }
    QMatrix4x4 rotation() const;

    Camera toCamera() const;

    // Transform.
    QVector3D project(const QVector3D &world) const;
    QVector3D unproject(const QVector3D &window) const;
    void ray(int x, int y, QVector3D *near, QVector3D *far);

    // Interaction.
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void rotate(int dx, int dy);
    void pan(int dx, int dy);
    void zoom(int dy);
    void setOffset(const QVector3D &offset) { offset_ = offset; }

    void setLock2d(bool b) { lock2d_ = b; }
    bool lock2d() const { return lock2d_; }

    // Matrix.
    const QMatrix4x4 &modelView() const { return modelView_; }
    const QMatrix4x4 &projection() const { return projection_; }
    const QMatrix4x4 &modelViewProjection() const
    {
        return modelViewProjection_;
    }

    const QMatrix4x4 &modelViewInv() const { return modelViewInv_; }
    const QMatrix4x4 &projectionInv() const { return projectionInv_; }
    const QMatrix4x4 &modelViewProjectionInv() const
    {
        return modelViewProjectionInv_;
    }

protected:
    // Camera.
    QVector3D eye_;
    QVector3D center_;
    QVector3D offset_;
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
    bool lock2d_;

    void updateMatrix();
    void setModelView(const QMatrix4x4 &m);
    void setProjection(const QMatrix4x4 &m);
    void setModelViewProjection(const QMatrix4x4 &m);
    void updateFrustrum();

    friend void toJson(Json &out, const ViewerCamera &in);
    friend std::string toString(const ViewerCamera &in);
    friend std::ostream &operator<<(std::ostream &out, const ViewerCamera &in);
};

inline void toJson(Json &out, const ViewerCamera &in)
{
    toJson(out["eye"], in.eye_);
    toJson(out["center"], in.center_);
    toJson(out["right"], in.right_);
    toJson(out["up"], in.up_);
    toJson(out["direction"], in.direction_);
    toJson(out["distance"], in.distance_);
}

inline std::string toString(const ViewerCamera &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

inline std::string toString(const QMatrix4x4 &in)
{
    return "[[" + std::to_string(in(0, 0)) + ", " + std::to_string(in(0, 1)) +
           ", " + std::to_string(in(0, 2)) + ", " + std::to_string(in(0, 3)) +
           "], [" + std::to_string(in(1, 0)) + ", " + std::to_string(in(1, 1)) +
           ", " + std::to_string(in(1, 2)) + ", " + std::to_string(in(1, 3)) +
           "], [" + std::to_string(in(2, 0)) + ", " + std::to_string(in(2, 1)) +
           ", " + std::to_string(in(2, 2)) + ", " + std::to_string(in(2, 3)) +
           "], [" + std::to_string(in(3, 0)) + ", " + std::to_string(in(3, 1)) +
           ", " + std::to_string(in(3, 2)) + ", " + std::to_string(in(3, 3)) +
           "]]";
}

inline std::ostream &operator<<(std::ostream &out, const ViewerCamera &in)
{
    return out << toString(in);
}

#endif /* VIEWER_CAMERA_HPP */
