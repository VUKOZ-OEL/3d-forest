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

/** @file QtViewerAabb.hpp */

#ifndef QT_VIEWER_AABB_HPP
#define QT_VIEWER_AABB_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Box.hpp>
#include <QtViewerUtil.hpp>

// Include Qt.
#include <QVector3D>

/** Qt Viewer Axis-Aligned Bounding Box. */
class QtViewerAabb
{
public:
    QtViewerAabb();
    ~QtViewerAabb();

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

    void extend(const QtViewerAabb &box);

    void invalidate();
    bool valid() const { return valid_; }

    const QVector3D &min() const { return min_; }
    const QVector3D &max() const { return max_; }

    const QVector3D &center() const { return center_; }
    float radius() const { return radius_; }

    void range(const QVector3D &eye,
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

    friend void toJson(Json &out, const QtViewerAabb &in);
    friend std::string toString(const QtViewerAabb &in);
    friend std::ostream &operator<<(std::ostream &out, const QtViewerAabb &in);
};

inline void toJson(Json &out, const QtViewerAabb &in)
{
    toJson(out["min"], in.min_);
    toJson(out["max"], in.max_);
    toJson(out["center"], in.center_);
    toJson(out["radius"], in.radius_);
    toJson(out["valid"], in.valid_);
}

inline std::string toString(const QtViewerAabb &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

inline std::ostream &operator<<(std::ostream &out, const QtViewerAabb &in)
{
    return out << toString(in);
}

#endif /* QT_VIEWER_AABB_HPP */