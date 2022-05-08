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

/** @file GuiAabb.hpp */

#ifndef GUI_AABB_HPP
#define GUI_AABB_HPP

#include <vector>

#include <Box.hpp>

#include <QVector3D>

/** Gui Axis-Aligned Bounding Box. */
class GuiAabb
{
public:
    GuiAabb();
    ~GuiAabb();

    void set(float min_x,
             float min_y,
             float min_z,
             float max_x,
             float max_y,
             float max_z);
    void set(const QVector3D &min, const QVector3D &max);
    void set(const std::vector<float> &xyz);
    void set(const Box<float> &box);
    void set(const Box<double> &box);

    void extend(const GuiAabb &box);

    void invalidate();
    bool isValid() const { return valid_; }

    const QVector3D &getMin() const { return min_; }
    const QVector3D &getMax() const { return max_; }

    const QVector3D &getCenter() const { return center_; }
    float getRadius() const { return radius_; }

    void getRange(const QVector3D &eye,
                  const QVector3D &direction,
                  float *min,
                  float *max);

protected:
    QVector3D min_;
    QVector3D max_;
    QVector3D center_;
    float radius_;
    bool valid_;

    void setValid();
};

#endif /* GUI_AABB_HPP */