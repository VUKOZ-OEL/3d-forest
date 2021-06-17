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

/** @file GL.hpp */

#ifndef GL_HPP
#define GL_HPP

#include <ClipFilter.hpp>
#include <GLAabb.hpp>
#include <QVector3D>
#include <vector>

/** OpenGL. */
class GL
{
public:
    enum Mode
    {
        POINTS,
        LINES,
        QUADS
    };

    GL();
    ~GL();

    static void render(Mode mode,
                       const std::vector<float> &xyz,
                       const std::vector<float> &rgb);

    static void render(Mode mode,
                       const std::vector<float> &xyz,
                       const std::vector<float> &rgb,
                       const std::vector<unsigned int> &indices);

    static void renderClipFilter(const ClipFilter &clipFilter);
    static void renderAabb(const GLAabb &box);
    static void renderAxis(const GLAabb &box, const QVector3D &center);

protected:
};

#endif /* GL_HPP */