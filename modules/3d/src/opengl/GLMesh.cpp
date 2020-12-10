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
    @file GLMesh.cpp
*/

#include <GLMesh.hpp>
#include <QOpenGLFunctions>

GLMesh::GLMesh() : mode(GLMesh::POINTS)
{
}

GLMesh::~GLMesh()
{
}

void GLMesh::render()
{
    validate();

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
            glColor3f(color[0], color[1], color[2]);
        }

        glDrawArrays(glmode, 0, n / 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    // Debug
    renderAabb();
}

void GLMesh::validate()
{
    if (!aabb_.isValid())
    {
        aabb_.set(xyz);
    }
}
