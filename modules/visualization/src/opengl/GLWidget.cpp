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

#include <GLMesh.hpp>
#include <GLViewer.hpp>
#include <GLWidget.hpp>
#include <MeshNode.hpp>
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    initializeGLWidget();
}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGLWidget()
{
    validateNodes();

    QVector3D eye(0.F, 0.F, 5.F);
    QVector3D center(0.F, 0.F, 0.F);
    QVector3D up(0.F, 1.F, 0.F);
    if (aabb_.isValid())
    {
        center = aabb_.getCenter();
        eye = center + QVector3D(0.F, 0.F, 2.F) * aabb_.getRadius();
    }
    camera_.setLookAt(eye, center, up);
}

void GLWidget::updateScene(const Scene &scene)
{
    (void)scene;

    nodes_.clear();
#if 0
    for (const auto &it : scene)
    {
        const MeshNode *mesh = dynamic_cast<const MeshNode *>(it.get());
        if (mesh)
        {
            std::shared_ptr<GLMesh> glmesh = std::make_shared<GLMesh>();
            glmesh->color = QVector3D(1.F, 1.F, 1.F);
            glmesh->mode = GLMesh::POINTS;
            glmesh->xyz = mesh->xyz;
            glmesh->rgb = mesh->rgb;
            nodes_.push_back(glmesh);
        }
    }
#endif
    validateNodes();
    resetCamera();
}

void GLWidget::updateScene(Editor *editor)
{
    (void)editor;

    nodes_.clear();
    validateNodes();
    resetCamera();
}

void GLWidget::resetCamera()
{
    QVector3D eye(0.F, 5.F, 0.F);
    QVector3D center(0.F, 0.F, 0.F);
    QVector3D up(0.F, 0.F, 1.F);
    if (aabb_.isValid())
    {
        center = aabb_.getCenter();
        eye = center + QVector3D(0.F, 1.F, 0.F) * aabb_.getRadius();
    }
    camera_.setLookAt(eye, center, up);
}

void GLWidget::setViewer(GLViewer *viewer)
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

void GLWidget::validateNodes()
{
    aabb_.invalidate();

    for (auto &node : nodes_)
    {
        node->validate();
        aabb_.extend(node->getAabb());
    }
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
}

void GLWidget::paintGL()
{
    if (isSelected())
    {
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
    }
    else
    {
        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup camera
    glViewport(0, 0, camera_.width(), camera_.height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.getProjection().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.getModelView().data());

    // Render nodes
    for (auto &node : nodes_)
    {
        node->render();
    }
}

void GLWidget::resizeGL(int w, int h)
{
    camera_.setViewport(0, 0, w, h);
    camera_.setPerspective(60.0F, 3.0F, 1000.0F);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (!isSelected())
    {
        if (viewer_)
        {
            viewer_->selectViewport(this);
        }
    }

    camera_.mousePressEvent(event);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    camera_.mouseMoveEvent(event);
    update();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (!isSelected())
    {
        if (viewer_)
        {
            viewer_->selectViewport(this);
        }
    }

    camera_.wheelEvent(event);
    update();
}
