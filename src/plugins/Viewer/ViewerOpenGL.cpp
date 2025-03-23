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

/** @file ViewerOpenGL.cpp */

// Include 3D Forest.
#include <Matrix4.hpp>
#include <Util.hpp>
#include <ViewerOpenGL.hpp>
#include <ViewerOpenGLManager.hpp>

// Include Qt.
#include <QMatrix4x4>

// Include local.
#define LOG_MODULE_NAME "ViewerOpenGL"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

const char *viewerOpenGLErrorString(GLenum err)
{
    switch (err)
    {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_STACK_OVERFLOW:
            return "Stack overflow";
        case GL_STACK_UNDERFLOW:
            return "Stack underflow";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Invalid framebuffer operation";
        default:
            return "Unknown error";
    }
}

ViewerOpenGL::ViewerOpenGL()
{
}

ViewerOpenGL::~ViewerOpenGL()
{
}

void ViewerOpenGL::render(Mode mode,
                          const float *position,
                          size_t positionSize,
                          const float *color,
                          size_t colorSize,
                          const float *normal,
                          size_t normalSize,
                          const unsigned int *indices,
                          size_t indicesSize)
{
    if (positionSize < 1)
    {
        return;
    }

    // Specify what kind of primitives to render.
    GLenum glmode;
    switch (mode)
    {
        case LINES:
            glmode = GL_LINES;
            break;
        case TRIANGLES:
            glmode = GL_TRIANGLES;
            break;
        case QUADS:
            glmode = GL_QUADS;
            break;
        case POINTS:
        default:
            glmode = GL_POINTS;
            break;
    }

    // Render.
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, position);

    if (colorSize > 0)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, color);
    }

    if (normalSize > 0)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, normal);
    }

    if (indicesSize > 0)
    {
        GLsizei n = static_cast<GLsizei>(indicesSize);
        LOG_DEBUG(<< "glDrawElements n <" << n << ">.");
        glDrawElements(glmode, n, GL_UNSIGNED_INT, indices);
    }
    else
    {
        GLsizei n = static_cast<GLsizei>(positionSize / 3);
        LOG_DEBUG(<< "glDrawArrays n <" << n << ">.");
        glDrawArrays(glmode, 0, n);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void ViewerOpenGL::render(const Mesh &mesh)
{
    Mode mode;
    if (mesh.mode == Mesh::Mode::MODE_LINES)
    {
        mode = LINES;
    }
    else if (mesh.mode == Mesh::Mode::MODE_TRIANGLES)
    {
        mode = TRIANGLES;
    }
    else
    {
        mode = POINTS;
    }

    ViewerOpenGL::render(mode,
                         mesh.position.data(),
                         mesh.position.size(),
                         mesh.color.data(),
                         mesh.color.size(),
                         mesh.normal.data(),
                         mesh.normal.size(),
                         mesh.indices.data(),
                         mesh.indices.size());
}

void ViewerOpenGL::renderClipFilter(const Region &clipFilter)
{
    if (clipFilter.shape == Region::Shape::NONE)
    {
        return;
    }

    glColor3f(0.0F, 0.75F, 0.0F);
    // glDepthMask(GL_FALSE);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xff);

    if (clipFilter.shape == Region::Shape::CYLINDER)
    {
        Vector3<float> a(clipFilter.cylinder.a());
        Vector3<float> b(clipFilter.cylinder.b());
        float radius = static_cast<float>((clipFilter.cylinder.radius()));

        ViewerOpenGL::renderCylinder(a, b, radius, 10);
    }
    else
    {
        ViewerAabb box;

        if (clipFilter.shape == Region::Shape::BOX)
        {
            box.set(clipFilter.box);
        }
        else
        {
            box.set(clipFilter.boundary);
        }

        ViewerOpenGL::renderAabb(box);
    }

    glDisable(GL_LINE_STIPPLE);
    // glDepthMask(GL_TRUE);
}

void ViewerOpenGL::renderAabb(const ViewerAabb &box)
{
    // Render wireframe cuboid.
    QVector3D v[8];
    QVector3D min = box.min();
    QVector3D max = box.max();

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

void ViewerOpenGL::renderCylinder(const Vector3<float> &a,
                                  const Vector3<float> &b,
                                  float radius,
                                  size_t slices)
{
    // Create wire-frame cylinder.
    Vector3<float> ab = b - a;
    float length = ab.length();

    if (length < 1e-6F || slices < 3)
    {
        return;
    }

    Vector3<float> n1 = ab.normalized();
    Vector3<float> n2 = n1.perpendicular();

    double sliceAngle = 6.283185307 / static_cast<double>(slices);

    GLuint nSlices = static_cast<GLuint>(slices);
    std::vector<Vector3<float>> xyz;
    std::vector<GLuint> indices;

    xyz.resize(slices * 2);
    indices.resize(slices * 6);

    for (GLuint i = 0; i < nSlices; i++)
    {
        n2.normalize();

        xyz[i * 2 + 0] = a + (radius * n2);
        xyz[i * 2 + 1] = b + (radius * n2);

        indices[i * 6 + 0] = i * 2;
        indices[i * 6 + 1] = i * 2 + 1;

        if (i + 1 == nSlices)
        {
            indices[i * 6 + 2] = i * 2;
            indices[i * 6 + 3] = 0;

            indices[i * 6 + 4] = i * 2 + 1;
            indices[i * 6 + 5] = 1;
        }
        else
        {
            indices[i * 6 + 2] = i * 2;
            indices[i * 6 + 3] = i * 2 + 2;

            indices[i * 6 + 4] = i * 2 + 1;
            indices[i * 6 + 5] = i * 2 + 3;
        }

        n2 = n2.rotated(n1, sliceAngle);
    }

    // Render the vertex array.
    GLsizei indicesCount = static_cast<GLsizei>(indices.size());
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(xyz.data()));
    glDrawElements(GL_LINES, indicesCount, GL_UNSIGNED_INT, indices.data());
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ViewerOpenGL::renderAxis()
{
    float d = 1.0F;

    QVector3D v[6];
    v[0] = QVector3D(0.0F, 0.0F, 0.0F);
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
    c[4] = QVector3D(0.0F, 0.3F, 1.0F);
    c[5] = QVector3D(0.0F, 0.3F, 1.0F);

    GLuint indices[6] = {0, 1, 2, 3, 4, 5};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glColorPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&c[0]));
    glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void ViewerOpenGL::renderCross(const Vector3<float> &p,
                               float lengthX,
                               float lengthY)
{
    QVector3D v[4];
    v[0] = QVector3D(p[0] - (lengthX * 0.5F), p[1], p[2]);
    v[1] = QVector3D(p[0] + (lengthX * 0.5F), p[1], p[2]);
    v[2] = QVector3D(p[0], p[1] - (lengthY * 0.5F), p[2]);
    v[3] = QVector3D(p[0], p[1] + (lengthY * 0.5F), p[2]);

    GLuint indices[4] = {0, 1, 2, 3};

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ViewerOpenGL::renderLine(const Vector3<float> &a, const Vector3<float> &b)
{
    QVector3D v[2];
    v[0] = QVector3D(a[0], a[1], a[2]);
    v[1] = QVector3D(b[0], b[1], b[2]);

    GLuint indices[2] = {0, 1};

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(&v[0]));
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ViewerOpenGL::renderCircle(const Vector3<float> &p,
                                float radius,
                                size_t pointCount)
{
    GLuint pointCountGL = static_cast<GLuint>(pointCount);
    std::vector<float> xyz;
    std::vector<GLuint> indices;

    xyz.resize(pointCount * 3);
    indices.resize(pointCount);

    for (GLuint i = 0; i < pointCountGL; i++)
    {
        double angle = (static_cast<double>(i) * 2.0 * 3.14) /
                       static_cast<double>(pointCountGL);

        xyz[i * 3 + 0] = p[0] + (radius * static_cast<float>(cos(angle)));
        xyz[i * 3 + 1] = p[1] + (radius * static_cast<float>(sin(angle)));
        xyz[i * 3 + 2] = p[2];

        indices[i] = i;
    }

    // Render the vertex array.
    GLsizei indicesCount = static_cast<GLsizei>(indices.size());
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, static_cast<GLvoid *>(xyz.data()));
    glDrawElements(GL_LINE_LOOP, indicesCount, GL_UNSIGNED_INT, indices.data());
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ViewerOpenGL::renderText(ViewerOpenGLManager *manager,
                              const ViewerCamera &camera,
                              const Vector3<float> &p,
                              const std::string &text,
                              float scale)
{
    GLuint displayList = manager->font();
    if (displayList < 1)
    {
        return;
    }

    std::string cleanText(manager->cleanText(text));

    QMatrix4x4 mvi = camera.modelViewInv();
    mvi(0, 3) = p[0];
    mvi(1, 3) = p[1];
    mvi(2, 3) = p[2];

    SAFE_GL(glPushMatrix());
    SAFE_GL(glMultMatrixf(mvi.data()));
    glScalef(scale, scale, scale);
    glListBase(displayList);
    glCallLists(static_cast<GLsizei>(cleanText.size()),
                GL_BYTE,
                reinterpret_cast<const GLbyte *>(cleanText.c_str()));
    SAFE_GL(glPopMatrix());
}
