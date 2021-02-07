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
    @file GLNode.cpp
*/

#include <GLNode.hpp>
#include <QOpenGLFunctions>

GLNode::GLNode()
{
}

GLNode::~GLNode()
{
}

void GLNode::render()
{
}

void GLNode::validate()
{
}

void GLNode::renderAabb()
{
    // Render wireframe cuboid
    QVector3D v[8];
    QVector3D min = aabb_.getMin();
    QVector3D max = aabb_.getMax();

    v[0] = QVector3D(min[0], min[1], min[2]);
    v[1] = QVector3D(max[0], min[1], min[2]);
    v[2] = QVector3D(max[0], max[1], min[2]);
    v[3] = QVector3D(min[0], max[1], min[2]);
    v[4] = QVector3D(min[0], min[1], max[2]);
    v[5] = QVector3D(max[0], min[1], max[2]);
    v[6] = QVector3D(max[0], max[1], max[2]);
    v[7] = QVector3D(min[0], max[1], max[2]);

    GLuint indices[24] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                          6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    glColor3f(0.F, 1.F, 0.F);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
}
