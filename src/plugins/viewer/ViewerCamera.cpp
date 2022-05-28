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

/** @file ViewerCamera.cpp */

#include <cmath>

#include <ViewerCamera.hpp>

#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

ViewerCamera::ViewerCamera()
    : eye_(0.0F, 0.0F, 1.0F),
      center_(0.0F, 0.0F, 0.0F),
      right_(1.0F, 0.0F, 0.0F),
      up_(0.0F, 1.0F, 0.0F),
      direction_(0.0F, 0.0F, 1.0F),
      rotation_(),
      distance_(1.0F),
      fov_(60.0F),
      zNear_(0.01F),
      zFar_(100000.0F),
      perspective_(true),
      viewport_(0, 0, 100, 100),
      sensitivityX_(1.0F),
      sensitivityY_(1.0F),
      sensitivityZoom_(4.0F)
{
    frustrumPlanes_.resize(24);
    std::fill(frustrumPlanes_.begin(), frustrumPlanes_.end(), 0.0F);
    setOrthographic();
}

ViewerCamera::~ViewerCamera()
{
}

void ViewerCamera::setViewport(int x, int y, int width, int height)
{
    if (width < 1)
    {
        width = 1;
    }

    if (height < 1)
    {
        height = 1;
    }

    viewport_.setRect(x, y, width, height);
}

void ViewerCamera::updateProjection()
{
    if (perspective_)
    {
        setPerspective();
    }
    else
    {
        setOrthographic();
    }
}

void ViewerCamera::setPerspective()
{
    perspective_ = true;

    float zNear = zNear_ * distance_;
    float zFar = zFar_ * distance_;
    setPerspective(fov_, zNear, zFar);
}

void ViewerCamera::setOrthographic()
{
    perspective_ = false;

    // Aspect ratio
    float w = static_cast<float>(viewport_.width());
    float h = static_cast<float>(viewport_.height());
    float aspect = w / h;

    // Orthographic arguments
    float zNear = zNear_ * distance_;
    float zFar = zFar_ * distance_;

    QMatrix4x4 m;
    m.perspective(fov_, aspect, zNear, zFar);
    float w2 = -m(2, 3) * 28.9F;
    float left = -aspect * w2;
    float right = aspect * w2;
    float top = w2;
    float bottom = -w2;

    zNear = -zFar_ * distance_;

    // Projection matrix
    m.setToIdentity();
    m.ortho(left, right, bottom, top, zNear, zFar);
    setProjection(m);
}

void ViewerCamera::setPerspective(float fovy, float zNear, float zFar)
{
    float w = static_cast<float>(viewport_.width());
    float h = static_cast<float>(viewport_.height());
    float aspect = w / h;
    setPerspective(fovy, aspect, zNear, zFar);
}

void ViewerCamera::setPerspective(float fovy,
                                  float aspect,
                                  float zNear,
                                  float zFar)
{
    QMatrix4x4 m;
    m.perspective(fovy, aspect, zNear, zFar);
    setProjection(m);
}

void ViewerCamera::setLookAt(const QVector3D &center, float distance)
{
    center_ = center;
    rotation_ = QQuaternion();

    setDistance(distance);
}

void ViewerCamera::setLookAt(const QVector3D &eye,
                             const QVector3D &center,
                             const QVector3D &up)
{
    center_ = center;

    QVector3D dir = center_ - eye;
    distance_ = dir.length();
    dir.normalize();

    QVector3D right = QVector3D::crossProduct(dir, up);
    QVector3D u = QVector3D::crossProduct(right, dir);

    QMatrix3x3 m;
    m(0, 0) = right.x();
    m(0, 1) = right.y();
    m(0, 2) = right.z();
    m(1, 0) = u.x();
    m(1, 1) = u.y();
    m(1, 2) = u.z();
    m(2, 0) = -dir.x();
    m(2, 1) = -dir.y();
    m(2, 2) = -dir.z();
    rotation_ = QQuaternion::fromRotationMatrix(m);
    rotation_.normalize();

    updateProjection();
    updateMatrix();
}

void ViewerCamera::setDistance(float distance)
{
    if (distance < 1.0F)
    {
        distance = 1.0F;
    }
    distance_ = distance;

    updateProjection();
    updateMatrix();
}

Camera ViewerCamera::toCamera() const
{
    Camera ret;

    ret.eye.set(eye_.x(), eye_.y(), eye_.z());
    ret.center.set(center_.x(), center_.y(), center_.z());
    ret.up.set(up_.x(), up_.y(), up_.z());
    ret.fov = fov_;

    return ret;
}

QVector3D ViewerCamera::project(const QVector3D &world) const
{
    QVector3D p = world.project(modelView_, projection_, viewport_);
    p.setY(static_cast<float>(viewport_.bottom()) - p.y());
    return p;
}

QVector3D ViewerCamera::unproject(const QVector3D &window) const
{
    QVector3D p = window;
    p.setY(static_cast<float>(viewport_.bottom()) - p.y());
    return p.unproject(modelView_, projection_, viewport_);
}

void ViewerCamera::getRay(int x, int y, QVector3D *base, QVector3D *direction)
{
    QVector3D pt;
    int w = viewport_.right();
    int h = viewport_.bottom();
    y = h - y;
    if (w > 0 && h > 0)
    {
        pt[0] = ((2.0F * static_cast<float>(x)) / static_cast<float>(w)) - 1.0F;
        pt[1] = ((2.0F * static_cast<float>(y)) / static_cast<float>(h)) - 1.0F;
    }
    pt[2] = -1.0F;
    QVector3D p1 = projectionInv_.map(pt);
    pt[2] = 1.0F;
    QVector3D p2 = projectionInv_.map(pt);
    *base = modelViewInv_.map(p1);

    QVector3D viewdirection = p2 - p1;
    viewdirection.normalize();
    *direction = modelViewInv_.mapVector(viewdirection);
    direction->normalize();
}

void ViewerCamera::mousePressEvent(QMouseEvent *event)
{
    mouseLastPosition_ = event->pos();
}

void ViewerCamera::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->pos().x() - mouseLastPosition_.x();
    int dy = event->pos().y() - mouseLastPosition_.y();

    if (event->buttons() & Qt::LeftButton)
    {
        rotate(dx, dy);
    }
    else if (event->buttons() & Qt::RightButton)
    {
        pan(dx, dy);
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        zoom(dy);
    }

    mouseLastPosition_ = event->pos();
}

void ViewerCamera::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numPixels.isNull())
    {
        zoom(numPixels.y());
    }
    else if (!numDegrees.isNull())
    {
        QPoint numSteps = numDegrees;
        zoom(-(numSteps.y()));
    }

    event->accept();
}

void ViewerCamera::rotate(int dx, int dy)
{
    float x = static_cast<float>(dx) / static_cast<float>(viewport_.width());
    float y = static_cast<float>(dy) / static_cast<float>(viewport_.height());

    float az = sensitivityX_ * x * 180.0F;
    float ax = sensitivityY_ * y * 180.0F;

    QQuaternion qx = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), ax);
    QQuaternion qz = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), az);

    rotation_ = qx * rotation_ * qz;
    rotation_.normalize();

    updateMatrix();
}

void ViewerCamera::pan(int dx, int dy)
{
    float x = static_cast<float>(dx) / static_cast<float>(viewport_.width());
    float y = static_cast<float>(-dy) / static_cast<float>(viewport_.height());

    x *= sensitivityX_ * distance_;
    y *= sensitivityY_ * distance_;

    QVector3D shift = (x * right_) + (y * up_);
    center_ -= shift;

    updateMatrix();
}

void ViewerCamera::zoom(int dy)
{
    float d = static_cast<float>(dy) / static_cast<float>(viewport_.height());
    setDistance(distance_ * std::exp(sensitivityZoom_ * d));
}

QMatrix4x4 ViewerCamera::rotation() const
{
    QMatrix4x4 m;
    m.rotate(rotation_);
    return m;
}

void ViewerCamera::updateMatrix()
{
    QMatrix4x4 m;
    m.translate(0, 0, -distance_);
    m.rotate(rotation_);
    m.translate(-center_);
    setModelView(m);

    up_ = QVector3D(m(1, 0), m(1, 1), m(1, 2));
    right_ = QVector3D(m(0, 0), m(0, 1), m(0, 2));
    direction_ = QVector3D(m(2, 0), m(2, 1), m(2, 2));
    eye_ = center_ + (direction_ * distance_);
}

void ViewerCamera::setModelView(const QMatrix4x4 &m)
{
    modelView_ = m;
    modelViewInv_ = m.inverted();
    setModelViewProjection(modelView_ * projection_);
}

void ViewerCamera::setProjection(const QMatrix4x4 &m)
{
    projection_ = m;
    projectionInv_ = m.inverted();
    setModelViewProjection(modelView_ * projection_);
}

void ViewerCamera::setModelViewProjection(const QMatrix4x4 &m)
{
    modelViewProjection_ = m;
    modelViewProjectionInv_ = m.inverted();
    updateFrustrum();
}

void ViewerCamera::updateFrustrum()
{
    const float *m = modelViewProjection_.data();

    // Extract the planes
    frustrumPlanes_[0] = m[3] - m[0];
    frustrumPlanes_[1] = m[7] - m[4];
    frustrumPlanes_[2] = m[11] - m[8];
    frustrumPlanes_[3] = m[15] - m[12];

    frustrumPlanes_[4] = m[3] + m[0];
    frustrumPlanes_[5] = m[7] + m[4];
    frustrumPlanes_[6] = m[11] + m[8];
    frustrumPlanes_[7] = m[15] + m[12];

    frustrumPlanes_[8] = m[3] + m[1];
    frustrumPlanes_[9] = m[7] + m[5];
    frustrumPlanes_[10] = m[11] + m[9];
    frustrumPlanes_[11] = m[15] + m[13];

    frustrumPlanes_[12] = m[3] - m[1];
    frustrumPlanes_[13] = m[7] - m[5];
    frustrumPlanes_[14] = m[11] - m[9];
    frustrumPlanes_[15] = m[15] - m[13];

    frustrumPlanes_[16] = m[3] - m[2];
    frustrumPlanes_[17] = m[7] - m[6];
    frustrumPlanes_[18] = m[11] - m[10];
    frustrumPlanes_[19] = m[15] - m[14];

    frustrumPlanes_[20] = m[3] + m[2];
    frustrumPlanes_[21] = m[7] + m[6];
    frustrumPlanes_[22] = m[11] + m[10];
    frustrumPlanes_[23] = m[15] + m[14];

    // Normalize
    size_t n = 0;
    float norm;
    for (size_t i = 0; i < 6; i++)
    {
        norm = static_cast<float>(
            std::sqrt(frustrumPlanes_[n + 0] * frustrumPlanes_[n] +
                      frustrumPlanes_[n + 1] * frustrumPlanes_[n + 1] +
                      frustrumPlanes_[n + 2] * frustrumPlanes_[n + 2] +
                      frustrumPlanes_[n + 3] * frustrumPlanes_[n + 3]));

        constexpr float e = std::numeric_limits<float>::epsilon();
        if (norm > e)
        {
            frustrumPlanes_[n + 0] /= norm;
            frustrumPlanes_[n + 1] /= norm;
            frustrumPlanes_[n + 2] /= norm;
            frustrumPlanes_[n + 3] /= norm;
        }
        n += 4;
    }
}
