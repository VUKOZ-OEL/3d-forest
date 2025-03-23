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

/** @file ViewerOpenGLStrokeFont.hpp */

#ifndef VIEWER_OPEN_GL_STROKE_FONT_HPP
#define VIEWER_OPEN_GL_STROKE_FONT_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <ViewerOpenGL.hpp>
#include <ViewerOpenGLResource.hpp>

/** Viewer OpenGL Stroke Font. */
class ViewerOpenGLStrokeFont : public ViewerOpenGLResource
{
public:
    ViewerOpenGLStrokeFont();
    ~ViewerOpenGLStrokeFont();

    /** Get base of the display list representation. */
    GLuint list(ViewerOpenGLManager *managerGL);

    float textWidth(const std::string &text);

    float textHeight();

    /** Update new geometry in OpenGL. */
    void update();

protected:
    /** New build. */
    virtual void updateGL(ViewerOpenGLManager *managerGL);

    void createStrokeFont(GLuint listId);

    size_t nChars_;
    std::vector<float> charWidth_;
    float charHeight_;

    // resources
    static const size_t displayListId_ = 0;
};

#endif /* VIEWER_OPEN_GL_STROKE_FONT_HPP */