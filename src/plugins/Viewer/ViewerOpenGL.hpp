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

/** @file ViewerOpenGL.hpp */

#ifndef VIEWER_OPENGL_HPP
#define VIEWER_OPENGL_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Mesh.hpp>
#include <Region.hpp>
#include <ViewerAabb.hpp>
#include <ViewerCamera.hpp>
class ViewerOpenGLManager;

// Include Qt.
#include <QOpenGLFunctions>
#include <QVector3D>

extern const char *viewerOpenGLErrorString(GLenum err);

#if !defined(DEBUG_MODE) /* Release */
    //! Tests for OpenGL errors are ignored.
    #define SAFE_GL(cmd) cmd;
#else /* DEBUG mode */
    //! Tests OpenGL error (glGetError()) after glCommand (cmd).
    #define SAFE_GL(cmd)                                                       \
        cmd;                                                                   \
        {                                                                      \
            GLenum error;                                                      \
            while ((error = glGetError()) != GL_NO_ERROR)                      \
            {                                                                  \
                LOG_ERROR(<< "OpenGL error <"                                  \
                          << viewerOpenGLErrorString(error) << "> in <"        \
                          << (#cmd) << ">.");                                  \
            }                                                                  \
        }
#endif /* DEBUG_MODE */

//! Always test the errors.
#define CHECK_GL()                                                             \
    {                                                                          \
        GLenum error;                                                          \
        while ((error = glGetError()) != GL_NO_ERROR)                          \
        {                                                                      \
            LOG_ERROR(<< "OpenGL error <" << viewerOpenGLErrorString(error)    \
                      << ">.");                                                \
        }                                                                      \
    }

/** Viewer OpenGL. */
class ViewerOpenGL
{
public:
    enum Mode
    {
        POINTS,
        LINES,
        TRIANGLES,
        QUADS
    };

    typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum);
    typedef void (*PFNGLDRAWRANGEELEMENTSPROC)(GLenum mode,
                                               GLuint start,
                                               GLuint end,
                                               GLsizei count,
                                               GLenum type,
                                               const GLvoid *indices);

    typedef void (*PFNGLPROGRAMSTRINGARBPROC)(GLenum target,
                                              GLenum format,
                                              GLsizei len,
                                              const GLvoid *string);
    typedef void (*PFNGLBINDPROGRAMARBPROC)(GLenum target, GLuint program);
    typedef void (*PFNGLDELETEPROGRAMSARBPROC)(GLsizei n,
                                               const GLuint *programs);
    typedef void (*PFNGLGENPROGRAMSARBPROC)(GLsizei n, GLuint *programs);
    typedef void (*PFNGLPROGRAMENVPARAMETER4FARBPROC)(GLenum target,
                                                      GLuint index,
                                                      GLfloat x,
                                                      GLfloat y,
                                                      GLfloat z,
                                                      GLfloat w);
    typedef void (*PFNGLPROGRAMENVPARAMETER4FVARBPROC)(GLenum target,
                                                       GLuint index,
                                                       const GLfloat *params);

    typedef void (*PFNGLBINDBUFFERARBPROC)(GLenum target, GLuint buffer);
    typedef void (*PFNGLDELETEBUFFERSARBPROC)(GLsizei n, const GLuint *buffers);
    typedef void (*PFNGLGENBUFFERSARBPROC)(GLsizei n, GLuint *buffers);
    typedef GLboolean (*PFNGLISBUFFERARBPROC)(GLuint buffer);
    typedef void (*PFNGLBUFFERDATAARBPROC)(GLenum target,
                                           GLsizeiptrARB size,
                                           const GLvoid *data,
                                           GLenum usage);

    enum ARB
    {
        VERTEX_PROGRAM_ARB = 0x8620,
        FRAGMENT_PROGRAM_ARB = 0x8804,
        PROGRAM_FORMAT_ASCII_ARB = 0x8875,
        PROGRAM_ERROR_POSITION_ARB = 0x864B,
        PROGRAM_ERROR_STRING_ARB = 0x8874,

        ARRAY_BUFFER_ARB = 0x8892,
        ELEMENT_ARRAY_BUFFER_ARB = 0x8893,
        STATIC_DRAW_ARB = 0x88E4,
        MAX_ELEMENTS_VERTICES = 0x80E8,
        MAX_ELEMENTS_INDICES = 0x80E9,

        POINT_SPRITE_ARB = 0x8861,
        COORD_REPLACE_ARB = 0x8862,
        VERTEX_PROGRAM_POINT_SIZE_ARB = 0x8642
    };

    enum GL12
    {
        // separate specular color
        LIGHT_MODEL_COLOR_CONTROL = 0x81F8,
        SINGLE_COLOR = 0x81F9,
        SEPARATE_SPECULAR_COLOR = 0x81FA
    };

    ViewerOpenGL();
    ~ViewerOpenGL();

    static void render(Mode mode,
                       const float *position,
                       size_t positionSize,
                       const float *color,
                       size_t colorSize,
                       const float *normal,
                       size_t normalSize,
                       const unsigned int *indices,
                       size_t indicesSize);

    static void render(const Mesh &mesh);

    static void renderClipFilter(const Region &clipFilter);
    static void renderAabb(const ViewerAabb &box);
    static void renderAabbCorners(const ViewerAabb &box, float scale = 0.025F);
    static void renderCylinder(const Vector3<float> &a,
                               const Vector3<float> &b,
                               float radius,
                               size_t slices = 16);
    static void renderHollowCylinder(const Vector3<float> &a,
                                     const Vector3<float> &b,
                                     float radius,
                                     size_t slices = 16);
    static void renderAxis();
    static void renderCross(const Vector3<float> &p,
                            float lengthX,
                            float lengthY);
    static void renderLine(const Vector3<float> &a, const Vector3<float> &b);
    static void renderCircle(const Vector3<float> &p,
                             float radius,
                             size_t pointCount = 16);

    static void renderText(ViewerOpenGLManager *manager,
                           const ViewerCamera &camera,
                           const Vector3<float> &p,
                           const std::string &text,
                           float scale = 1.0F);

protected:
};

#endif /* VIEWER_OPENGL_HPP */