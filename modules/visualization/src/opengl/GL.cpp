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
    @file GL.cpp
*/

#include <GL.hpp>
#include <QOpenGLFunctions>

GL::GL()
{
}

GL::~GL()
{
}

void GL::render(Mode mode,
                const std::vector<float> &xyz,
                const std::vector<float> &rgb)
{
    // Specify what kind of primitives to render
    GLenum glmode;
    switch (mode)
    {
        case LINES:
            glmode = GL_LINES;
            break;
        case QUADS:
            glmode = GL_QUADS;
            break;
        case POINTS:
        default:
            glmode = GL_POINTS;
            break;
    }

    // Render
    GLsizei n = static_cast<GLsizei>(xyz.size());
    if (n > 0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, xyz.data());

        if (!rgb.empty())
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_FLOAT, 0, rgb.data());
        }
        else
        {
            glColor3f(1.0F, 1.0F, 1.0F);
        }

        glDrawArrays(glmode, 0, n / 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

void GL::render(Mode mode,
                const std::vector<float> &xyz,
                const std::vector<float> &rgb,
                const std::vector<unsigned int> &indices)
{
    // Specify what kind of primitives to render
    GLenum glmode;
    switch (mode)
    {
        case LINES:
            glmode = GL_LINES;
            break;
        case QUADS:
            glmode = GL_QUADS;
            break;
        case POINTS:
        default:
            glmode = GL_POINTS;
            break;
    }

    // Render
    GLsizei n = static_cast<GLsizei>(indices.size());
    if (n > 0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, xyz.data());

        if (!rgb.empty())
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_FLOAT, 0, rgb.data());
        }
        else
        {
            glColor3f(1.0F, 1.0F, 1.0F);
        }

        glDrawElements(glmode, n, GL_UNSIGNED_INT, indices.data());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

void GL::renderClipFilter(const ClipFilter &clipFilter)
{
    if (!clipFilter.enabled)
    {
        return;
    }

    GLAabb box;
    box.set(clipFilter.boxView);

    glClearColor(0.5F, 1.0F, 0.5F, 0.0F);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(4, 0xaaaa);

    GL::renderAabb(box);

    glDisable(GL_LINE_STIPPLE);
}

void GL::renderAabb(const GLAabb &box)
{
    // Render wireframe cuboid
    QVector3D v[8];
    QVector3D min = box.getMin();
    QVector3D max = box.getMax();

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

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void GL::renderAxis(const GLAabb &box, const QVector3D &center)
{
    float d = box.getRadius() * 0.5F;

    QVector3D v[6];
    v[0] = center;
    v[1] = QVector3D(v[0].x() + d, v[0].y(), v[0].z());
    v[2] = v[0];
    v[3] = QVector3D(v[0].x(), v[0].y() + d, v[0].z());
    v[4] = v[0];
    v[5] = QVector3D(v[0].x(), v[0].y(), v[0].z() + d);

    QVector3D c[6];
    c[0] = QVector3D(1.0F, 0.0F, 0.0F);
    c[1] = QVector3D(1.0F, 0.0F, 0.0F);
    c[2] = QVector3D(0.0F, 1.0F, 0.0F);
    c[3] = QVector3D(0.0F, 1.0F, 0.0F);
    c[4] = QVector3D(0.0F, 0.0F, 1.0F);
    c[5] = QVector3D(0.0F, 0.0F, 1.0F);

    GLuint indices[6] = {0, 1, 2, 3, 4, 5};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glColorPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&c[0]));
    glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}
