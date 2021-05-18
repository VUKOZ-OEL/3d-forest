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
    @file GLWidget.cpp
*/

#include <Editor.hpp>
#include <GL.hpp>
#include <GLWidget.hpp>
#include <QMouseEvent>
#include <QWheelEvent>
#include <Time.hpp>
#include <Viewer.hpp>

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    viewer_ = nullptr;
    selected_ = false;
    editor_ = nullptr;

    resetCamera();
}

GLWidget::~GLWidget()
{
}

void GLWidget::setViewer(Viewer *viewer)
{
    viewer_ = viewer;
}

void GLWidget::setSelected(bool selected)
{
    selected_ = selected;
}

bool GLWidget::isSelected() const
{
    return selected_;
}

void GLWidget::updateScene(Editor *editor)
{
    editor_ = editor;
}

void GLWidget::resetScene(Editor *editor)
{
    aabb_.set(editor->boundaryView());
    resetCamera();
}

Camera GLWidget::camera() const
{
    return camera_.toCamera();
}

void GLWidget::setViewOrthographic()
{
    camera_.setOrthographic();
    cameraChanged(true);
}

void GLWidget::setViewPerspective()
{
    camera_.setPerspective();
    cameraChanged(true);
}

void GLWidget::setViewDirection(const QVector3D &dir, const QVector3D &up)
{
    QVector3D center = camera_.getCenter();
    float distance = camera_.getDistance();

    QVector3D eye = (dir * distance) + center;
    camera_.setLookAt(eye, center, up);
    cameraChanged(true);
}

void GLWidget::setViewTop()
{
    QVector3D dir(0.0F, 0.0F, 1.0F);
    QVector3D up(0.0F, 1.0F, 0.0F);
    setViewDirection(dir, up);
}

void GLWidget::setViewFront()
{
    QVector3D dir(0.0F, -1.0F, 0.0F);
    QVector3D up(0.0F, 0.0F, 1.0F);
    setViewDirection(dir, up);
}

void GLWidget::setViewLeft()
{
    QVector3D dir(-1.0F, 0.0F, 0.0F);
    QVector3D up(0.0F, 0.0F, 1.0F);
    setViewDirection(dir, up);
}

void GLWidget::setView3d()
{
    QVector3D dir(-1.0F, -1.0F, 1.0F);
    QVector3D up(1.065F, 1.0F, 1.0F);
    up.normalize();
    setViewDirection(dir, up);
}

void GLWidget::resetCamera()
{
    QVector3D center(0.0F, 0.0F, 0.0F);
    float distance = 1.0F;

    if (aabb_.isValid())
    {
        center = aabb_.getCenter();
        center[2] = aabb_.getMin().z();
        distance = aabb_.getRadius();
    }

    QVector3D eye(-1.0F, -1.0F, 1.0F);
    QVector3D up(1.065F, 1.0F, 1.0F);
    up.normalize();

    eye = (eye * distance) + center;
    camera_.setLookAt(eye, center, up);
}

void GLWidget::setViewResetDistance()
{
    QVector3D center = camera_.getCenter();
    QVector3D up = camera_.getUp();
    QVector3D dir = camera_.getDirection();

    float distance = 1.0F;
    if (aabb_.isValid())
    {
        distance = aabb_.getRadius();
    }

    QVector3D eye = (dir * distance) + center;
    camera_.setLookAt(eye, center, up);
    cameraChanged(true);
}

void GLWidget::setViewResetCenter()
{
    QVector3D dir = camera_.getDirection();
    QVector3D up = camera_.getUp();
    float distance = camera_.getDistance();

    QVector3D center = camera_.getCenter();
    if (aabb_.isValid())
    {
        center = aabb_.getCenter();
        center[2] = aabb_.getMin().z();
    }

    QVector3D eye = (dir * distance) + center;
    camera_.setLookAt(eye, center, up);
    cameraChanged(true);
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    setUpdateBehavior(QOpenGLWidget::PartialUpdate);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0F);
}

void GLWidget::paintGL()
{
    // Background
    if (isSelected())
    {
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    }
    else
    {
        glClearColor(0.1F, 0.1F, 0.1F, 0.0F);
    }

    // Setup camera
    glViewport(0, 0, camera_.width(), camera_.height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.getProjection().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.getModelView().data());

    // Render
    renderScene();

    GL::renderAabb(aabb_);
    //GL::renderAxis(aabb_, camera_.getCenter());
}

void GLWidget::renderScene()
{
    if (!editor_)
    {
        return;
    }

    editor_->lock();

    double t1 = getRealTime();

    size_t cellSize = editor_->database().cellSize();

    for (size_t cellIndex = 0; cellIndex < cellSize; cellIndex++)
    {
        DatabaseCell &cell = editor_->database().cell(cellIndex);

        if (cell.loaded && !cell.view.isFinished())
        {
            if (cellIndex == 0 && cell.view.isStarted())
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            GL::render(GL::POINTS, cell.view.xyz, cell.view.rgb);
            glFlush();

            cell.view.nextFrame();

            double t2 = getRealTime();
            if (t2 - t1 > 0.02 && cellIndex > 4)
            {
                break;
            }
        }
    }

    editor_->unlock();
}

void GLWidget::resizeGL(int w, int h)
{
    camera_.setViewport(0, 0, w, h);
    camera_.updateProjection();
    cameraChanged(true);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    camera_.mousePressEvent(event);
    setFocus();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    camera_.mouseMoveEvent(event);
    cameraChanged(true);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    camera_.wheelEvent(event);
    setFocus();
    cameraChanged(true);
}

void GLWidget::setFocus()
{
    if (!isSelected())
    {
        if (viewer_)
        {
            viewer_->selectViewport(this);
        }
    }
}

void GLWidget::cameraChanged(bool interactionFinished)
{
    if (viewer_)
    {
        emit viewer_->cameraChanged(interactionFinished);
    }
}
