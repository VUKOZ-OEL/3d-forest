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
    @file GLNode.hpp
*/

#ifndef GL_NODE_HPP
#define GL_NODE_HPP

#include <GLAabb.hpp>
#include <QMatrix4x4>
#include <QVector3D>

/** OpenGL Node. */
class GLNode
{
public:
    QVector3D color;
    QMatrix4x4 transformation;

    GLNode();
    virtual ~GLNode();

    virtual void render();
    virtual void validate();

    void invalidate() { aabb_.invalidate(); }

    const GLAabb &getAabb() const { return aabb_; }

protected:
    GLAabb aabb_;

    void renderAabb();
};

#endif /* GL_NODE_HPP */