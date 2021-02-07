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
    @file GLCamera.cpp
*/

#include <GLCamera.hpp>
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

GLCamera::GLCamera()
    : eye_(0.F, 0.F, 5.F), center_(0.F, 0.F, 0.F), up_(0.F, 1.F, 0.F),
      viewport_(0, 0, 100, 100)
{
    frustrumPlanes_.resize(24);
    std::fill(frustrumPlanes_.begin(), frustrumPlanes_.end(), 0.F);
}

GLCamera::~GLCamera()
{
}

void GLCamera::setViewport(int x, int y, int width, int height)
{
    viewport_.setRect(x, y, width, height);
}

void GLCamera::setPerspective(float fovy, float zNear, float zFar)
{
    float w = static_cast<float>(viewport_.width());
    float h = static_cast<float>(viewport_.height() ? viewport_.height() : 1);
    float aspect = w / h;
    setPerspective(fovy, aspect, zNear, zFar);
}

void GLCamera::setPerspective(float fovy, float aspect, float zNear, float zFar)
{
    QMatrix4x4 m;
    m.perspective(fovy, aspect, zNear, zFar);
    setProjection(m);
}

void GLCamera::setLookAt(const QVector3D &eye,
                         const QVector3D &center,
                         const QVector3D &up)
{
    eye_ = eye;
    center_ = center;
    up_ = up;

    QMatrix4x4 m;
    m.lookAt(eye_, center_, up_);
    setModelView(m);
}

void GLCamera::setDistance(float distance)
{
    QVector3D dir = eye_ - center_;
    dir.normalize();

    QVector3D eye = center_ + (dir * distance);

    setLookAt(eye, center_, up_);
}

float GLCamera::getDistance() const
{
    return (eye_ - center_).length();
}

QVector3D GLCamera::getDirection() const
{
    return getView().normalized();
}

QVector3D GLCamera::getView() const
{
    return center_ - eye_;
}

QVector3D GLCamera::getRight() const
{
    return QVector3D::crossProduct(getDirection(), up_);
}

QVector3D GLCamera::project(const QVector3D &world) const
{
    QVector3D p = world.project(modelView_, projection_, viewport_);
    p.setY(static_cast<float>(viewport_.bottom()) - p.y());
    return p;
}

QVector3D GLCamera::unproject(const QVector3D &window) const
{
    QVector3D p = window;
    p.setY(static_cast<float>(viewport_.bottom()) - p.y());
    return p.unproject(modelView_, projection_, viewport_);
}

void GLCamera::getRay(int x, int y, QVector3D *base, QVector3D *direction)
{
    QVector3D pt;
    int w = viewport_.right();
    int h = viewport_.bottom();
    y = h - y;
    if (w > 0 && h > 0)
    {
        pt[0] = ((2.F * static_cast<float>(x)) / static_cast<float>(w)) - 1.F;
        pt[1] = ((2.F * static_cast<float>(y)) / static_cast<float>(h)) - 1.F;
    }
    pt[2] = -1.F;
    QVector3D p1 = projectionInv_.map(pt);
    pt[2] = 1.0f;
    QVector3D p2 = projectionInv_.map(pt);
    *base = modelViewInv_.map(p1);

    QVector3D viewdirection = p2 - p1;
    viewdirection.normalize();
    *direction = modelViewInv_.mapVector(viewdirection);
    direction->normalize();
}

void GLCamera::mousePressEvent(QMouseEvent *event)
{
    mouseLastPosition_ = event->pos();
}

void GLCamera::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - mouseLastPosition_.x();
    int dy = event->y() - mouseLastPosition_.y();

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

void GLCamera::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numPixels.isNull())
    {
        zoom(numPixels.y());
    }
    else if (!numDegrees.isNull())
    {
        QPoint numSteps = numDegrees; // / 15;
        zoom(-(numSteps.y()));
    }

    event->accept();
}

void GLCamera::rotate(int dx, int dy)
{
    QVector3D orientation1;
    QVector3D orientation2;
    QVector3D eye;
    QVector3D center;
    QVector3D up;
    QVector3D right;
    float angleX;
    float angleY;
    float distance;

    eye = getEye();
    center = getCenter();
    up = getUp();
    angleX = static_cast<float>(-dx) * 0.005F;
    angleY = static_cast<float>(dy) * 0.005F;

    // X
    orientation1 = eye - center;
    distance = orientation1.length();
    orientation1.normalize();
    orientation2 = rotate(orientation1, up, angleX);
    eye = center + (orientation2 * distance);
    right = QVector3D::crossProduct(orientation2, up);

    // Y
    orientation1 = eye - center;
    distance = orientation1.length();
    orientation1.normalize();
    orientation2 = rotate(orientation1, right, angleY);
    eye = center + (orientation2 * distance);

    // Update up
    orientation1 = eye - center;
    orientation1.normalize();
    up = QVector3D::crossProduct(right, orientation1);

    // Force major axis, depends on ground orientation in scene data
    // TBD should not be parallel with 'orientation2'
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    right = QVector3D::crossProduct(orientation2, up);
    up = QVector3D::crossProduct(right, orientation2);

    setLookAt(eye, center, up);
}

void GLCamera::pan(int dx, int dy)
{
    float mx = static_cast<float>(dx) * 0.2F;
    float my = static_cast<float>(-dy) * 0.2F;

    QVector3D eye = getEye();
    QVector3D center = getCenter();
    QVector3D up = getUp();
    QVector3D right = getRight();
    QVector3D shift = (mx * right) + (my * up);

    eye -= shift;
    center -= shift;

    setLookAt(eye, center, up);
}

void GLCamera::zoom(int delta)
{
    float d = getDistance();
    d += static_cast<float>(delta) * 2.F;
    if (d < 1.F)
    {
        d = 1.F;
    }

    setDistance(d);
}

QVector3D GLCamera::rotate(const QVector3D &v,
                           const QVector3D &axis,
                           float angle)
{
    QVector3D v2 = axis.normalized();
    float w = cosf(-angle / 2.F);
    float s = sinf(-angle / 2.F);
    float xr = v2[0] * s;
    float yr = v2[1] * s;
    float zr = v2[2] * s;
    float w2 = w * w;
    float x2 = xr * xr;
    float y2 = yr * yr;
    float z2 = zr * zr;

    return QVector3D(
        v[0] * (w2 + x2 - y2 - z2) + v[1] * 2.F * (xr * yr + w * zr) +
            v[2] * 2.F * (xr * zr - w * yr),
        v[0] * 2.F * (xr * yr - w * zr) + v[1] * (w2 - x2 + y2 - z2) +
            v[2] * 2.F * (yr * zr + w * xr),
        v[0] * 2.F * (xr * zr + w * yr) + v[1] * 2.F * (yr * zr - w * xr) +
            v[2] * (w2 - x2 - y2 + z2));
}

void GLCamera::setModelView(const QMatrix4x4 &m)
{
    modelView_ = m;
    modelViewInv_ = m.inverted();
    setModelViewProjection(modelView_ * projection_);
}

void GLCamera::setProjection(const QMatrix4x4 &m)
{
    projection_ = m;
    projectionInv_ = m.inverted();
    setModelViewProjection(modelView_ * projection_);
}

void GLCamera::setModelViewProjection(const QMatrix4x4 &m)
{
    modelViewProjection_ = m;
    modelViewProjectionInv_ = m.inverted();
    updateFrustrum();
}

void GLCamera::updateFrustrum()
{
    const float *m = modelViewProjection_.data();

    // extract the planes
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

    // normalize
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
