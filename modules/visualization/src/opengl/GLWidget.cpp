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
#include <WindowViewports.hpp>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      windowViewports_(nullptr),
      viewportId_(0),
      selected_(false),
      editor_(nullptr)

{
    resetCamera();
}

GLWidget::~GLWidget()
{
}

void GLWidget::setWindowViewports(WindowViewports *viewer, size_t viewportId)
{
    windowViewports_ = viewer;
    viewportId_ = viewportId;
}

size_t GLWidget::viewportId() const
{
    return viewportId_;
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
}

void GLWidget::setViewPerspective()
{
    camera_.setPerspective();
}

void GLWidget::setViewDirection(const QVector3D &dir, const QVector3D &up)
{
    QVector3D center = camera_.getCenter();
    float distance = camera_.getDistance();

    QVector3D eye = (dir * distance) + center;
    camera_.setLookAt(eye, center, up);
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

void GLWidget::setViewRight()
{
    QVector3D dir(1.0F, 0.0F, 0.0F);
    QVector3D up(0.0F, 0.0F, 1.0F);
    setViewDirection(dir, up);
}

void GLWidget::setView3d()
{
    QVector3D dir(1.0F, -1.0F, 1.0F);
    QVector3D up(-1.065F, 1.0F, 1.0F);
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
        // center[2] = aabb_.getMin().z();
        distance = aabb_.getRadius();
    }

    QVector3D eye(1.0F, -1.0F, 1.0F);
    QVector3D up(-1.065F, 1.0F, 1.0F);
    eye.normalize();
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
        // center[2] = aabb_.getMin().z();
    }

    QVector3D eye = (dir * distance) + center;
    camera_.setLookAt(eye, center, up);
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
    glClearColor(0.2F, 0.2F, 0.2F, 0.0F);

    // Setup camera
    glViewport(0, 0, camera_.width(), camera_.height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.getProjection().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.getModelView().data());

    // Render
    renderScene();

    glColor3f(0.25F, 0.25F, 0.25F);
    GL::renderAabb(aabb_);
    GL::renderAxis(aabb_, camera_.getCenter());
}

void GLWidget::clearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLWidget::renderScene()
{
    if (!editor_)
    {
        return;
    }

    editor_->lock();

    const EditorSettings &settings = editor_->settings();
    glPointSize(settings.view().pointSize());

    double t1 = getRealTime();

    size_t tileViewSize = editor_->tileViewSize(viewportId_);

    if (tileViewSize == 0)
    {
        clearScreen();
    }

    for (size_t tileIndex = 0; tileIndex < tileViewSize; tileIndex++)
    {
        EditorTile &tile = editor_->tileView(viewportId_, tileIndex);

        if (tile.loaded && !tile.view.isFinished())
        {
            if (tileIndex == 0 && tile.view.isStarted())
            {
                clearScreen();
            }

            GL::render(GL::POINTS, tile.view.xyz, tile.view.rgb, tile.indices);
            glFlush();

            tile.view.nextFrame();

            double t2 = getRealTime();
            if (t2 - t1 > 0.02 && tileIndex > 4)
            {
                break;
            }
        }
    }

    GL::renderClipFilter(editor_->clipFilter());

    editor_->unlock();
}

void GLWidget::resizeGL(int w, int h)
{
    camera_.setViewport(0, 0, w, h);
    camera_.updateProjection();
    cameraChanged();
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
    cameraChanged();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    camera_.wheelEvent(event);
    setFocus();
    cameraChanged();
}

void GLWidget::setFocus()
{
    if (!isSelected())
    {
        if (windowViewports_)
        {
            windowViewports_->selectViewport(this);
        }
    }
}

void GLWidget::cameraChanged()
{
    if (windowViewports_)
    {
        emit windowViewports_->cameraChanged(viewportId_);
    }
}
