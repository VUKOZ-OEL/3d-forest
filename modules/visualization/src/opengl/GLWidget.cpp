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

/** @file GLWidget.cpp */

#include <Editor.hpp>
#include <GL.hpp>
#include <GLWidget.hpp>
#include <QDebug>
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

void GLWidget::resetScene(Editor *editor, bool resetView)
{
    aabb_.set(editor->boundaryView());
    if (resetView)
    {
        resetCamera();
    }
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
    dir.normalize();
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
        distance = aabb_.getRadius() * 2.0F;
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
        distance = aabb_.getRadius() * 2.0F;
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
    // Setup camera
    glViewport(0, 0, camera_.width(), camera_.height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.getProjection().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.getModelView().data());

    // Render
    bool firstFrame = renderScene();

    if (firstFrame)
    {
        renderGuides();
    }
}

void GLWidget::renderGuides()
{
    // Bounding box
    glColor3f(0.25F, 0.25F, 0.25F);
    GL::renderAabb(aabb_);

    // Overlay
    QMatrix4x4 m;
    float w = static_cast<float>(camera_.width());
    float h = static_cast<float>(camera_.height());

    m.ortho(-w * 0.5F, w * 0.5F, -h * 0.5F, h * 0.5F, -50.0F, 50.0F);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m.data());

    // Overlay: rotated
    float axesSize = 30.0F;
    m.setToIdentity();
    m.translate(-w * 0.5F + axesSize + 2.0F, -h * 0.5F + axesSize + 2.0F);
    m.scale(axesSize);
    m = m * camera_.rotation();

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m.data());

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0F);
    GL::renderAxis();
    glLineWidth(1.0F);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::clearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool GLWidget::renderScene()
{
    if (!editor_)
    {
        return true;
    }

    bool firstFrame = false;

    editor_->lock();

    renderSceneSettingsEnable();

    double t1 = getRealTime();

    size_t tileViewSize = editor_->tileViewSize(viewportId_);

    if (tileViewSize == 0)
    {
        clearScreen();
        firstFrame = true;
    }

    for (size_t tileIndex = 0; tileIndex < tileViewSize; tileIndex++)
    {
        EditorTile &tile = editor_->tileView(viewportId_, tileIndex);

        if (tile.renderMore())
        {
            if (tileIndex == 0 && tile.view.isStarted())
            {
                clearScreen();
                firstFrame = true;
            }

            GL::render(GL::POINTS, tile.view.xyz, tile.view.rgb, tile.indices);
            glFlush();

            tile.view.nextFrame();

            double t2 = getRealTime();
            if (t2 - t1 > 0.02)
            {
                break;
            }
        }
    }

    renderSceneSettingsDisable();

    if (firstFrame)
    {
        GL::renderClipFilter(editor_->clipFilter());
    }

    editor_->unlock();

    return firstFrame;
}

void GLWidget::renderSceneSettingsEnable()
{
    const EditorSettings &settings = editor_->settings();
    const EditorSettingsView &opt = settings.view();

    // Background
    const Vector3<float> &rgb = opt.background();
    glClearColor(rgb[0], rgb[1], rgb[2], 0.0F);

    // Point size
    glPointSize(opt.pointSize());

    // Fog
    if (opt.isFogEnabled())
    {
        QVector3D eye = camera_.getEye();
        QVector3D direction = -camera_.getDirection();
        direction.normalize();

        float min;
        float max;
        aabb_.getRange(eye, direction, &min, &max);
        float d = max - min;

        GLfloat colorFog[4]{0.0F, 0.0F, 0.0F, 0.0F};
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, colorFog);
        glHint(GL_FOG_HINT, GL_DONT_CARE);
        glFogf(GL_FOG_START, min);
        glFogf(GL_FOG_END, max + d * 0.1F);
        glEnable(GL_FOG);
    }
}

void GLWidget::renderSceneSettingsDisable()
{
    const EditorSettings &settings = editor_->settings();
    glPointSize(1.0F);

    if (settings.view().isFogEnabled())
    {
        glDisable(GL_FOG);
    }
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
