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
    @file GLAabb.hpp
*/

#ifndef GL_AABB_HPP
#define GL_AABB_HPP

#include <QVector3D>
#include <vector>

/** OpenGL AABB. */
class GLAabb
{
public:
    GLAabb();
    ~GLAabb();

    void set(float min_x,
             float min_y,
             float min_z,
             float max_x,
             float max_y,
             float max_z);
    void set(const QVector3D &min, const QVector3D &max);
    void set(const std::vector<float> &xyz);

    void extend(const GLAabb &box);

    void invalidate();
    bool isValid() const { return valid_; }

    const QVector3D &getMin() const { return min_; }
    const QVector3D &getMax() const { return max_; }

    const QVector3D &getCenter() const { return center_; }
    float getRadius() const { return radius_; }

protected:
    QVector3D min_;
    QVector3D max_;
    QVector3D center_;
    float radius_;
    bool valid_;

    void setValid();
};

#endif /* GL_AABB_HPP */