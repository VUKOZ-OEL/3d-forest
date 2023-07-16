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

/** @file Box.hpp */

#ifndef BOX_HPP
#define BOX_HPP

#include <Vector3.hpp>

#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Axis-Aligned Bounding Box. */
template <class T> class Box
{
public:
    Box();
    ~Box();

    template <class B> Box(const Box<B> &box);
    template <class B> Box(B x1, B y1, B z1, B x2, B y2, B z2);
    template <class B> Box(B cx, B cy, B cz, B r);
    template <class B> Box(const Vector3<B> &min, const Vector3<B> &max);

    void set(T x1, T y1, T z1, T x2, T y2, T z2);
    void set(T cx, T cy, T cz, T r);
    void set(const Vector3<T> &min, const Vector3<T> &max);
    void set(const std::vector<T> &xyz);
    void set(const Box<T> &box);
    void setPercent(const Box<T> &box, const Box<T> &a, const Box<T> &b);
    void translate(const Vector3<T> &v);

    void extend(const Box<T> &box);
    void extend(T x, T y, T z);
    void clear();

    bool empty() const { return empty_; }

    const T &min(size_t idx) const { return min_[idx]; }
    const T &max(size_t idx) const { return max_[idx]; }

    T length(size_t idx) const { return max_[idx] - min_[idx]; }
    T maximumLength() const;

    void getCenter(T &x, T &y, T &z) const;
    Vector3<T> getCenter() const;
    T distance(T x, T y, T z) const;
    T radius() const;

    bool intersects(const Box<T> &box) const;
    bool isInside(const Box<T> &box) const;
    bool isInside(T x, T y, T z) const;

    void read(const Json &in);
    Json &write(Json &out) const;

private:
    T min_[3];
    T max_[3];
    bool empty_;

    void validate();
};

template <class T> inline Box<T>::Box()
{
    clear();
}

template <class T> inline Box<T>::~Box()
{
}

template <class T> template <class B> inline Box<T>::Box(const Box<B> &box)
{
    min_[0] = static_cast<T>(box.min(0));
    min_[1] = static_cast<T>(box.min(1));
    min_[2] = static_cast<T>(box.min(2));
    max_[0] = static_cast<T>(box.max(0));
    max_[1] = static_cast<T>(box.max(1));
    max_[2] = static_cast<T>(box.max(2));

    validate();
}

template <class T>
template <class B>
inline Box<T>::Box(B x1, B y1, B z1, B x2, B y2, B z2)
{
    set(x1, y1, z1, x2, y2, z2);
}

template <class T> template <class B> inline Box<T>::Box(B cx, B cy, B cz, B r)
{
    set(cx, cy, cz, r);
}

template <class T>
template <class B>
inline Box<T>::Box(const Vector3<B> &min, const Vector3<B> &max)
{
    set(min, max);
}

template <class T> inline void Box<T>::set(T cx, T cy, T cz, T r)
{
    set(cx - r, cy - r, cz - r, cx + r, cy + r, cz + r);
}

template <class T>
inline void Box<T>::set(const Vector3<T> &min, const Vector3<T> &max)
{
    set(min[0], min[1], min[2], max[0], max[1], max[2]);
}

template <class T> inline void Box<T>::set(T x1, T y1, T z1, T x2, T y2, T z2)
{
    if (x1 > x2)
    {
        min_[0] = x2;
        max_[0] = x1;
    }
    else
    {
        min_[0] = x1;
        max_[0] = x2;
    }

    if (y1 > y2)
    {
        min_[1] = y2;
        max_[1] = y1;
    }
    else
    {
        min_[1] = y1;
        max_[1] = y2;
    }

    if (z1 > z2)
    {
        min_[2] = z2;
        max_[2] = z1;
    }
    else
    {
        min_[2] = z1;
        max_[2] = z2;
    }

    validate();
}

template <class T> inline void Box<T>::set(const std::vector<T> &xyz)
{
    size_t n = xyz.size() / 3;
    if (n > 0)
    {
        min_[0] = xyz[0];
        min_[1] = xyz[1];
        min_[2] = xyz[2];
        max_[0] = xyz[0];
        max_[1] = xyz[1];
        max_[2] = xyz[2];

        for (size_t i = 1; i < n; i++)
        {
            if (xyz[3 * i + 0] < min_[0])
                min_[0] = xyz[3 * i + 0];
            else if (xyz[3 * i + 0] > max_[0])
                max_[0] = xyz[3 * i + 0];
            if (xyz[3 * i + 1] < min_[1])
                min_[1] = xyz[3 * i + 1];
            else if (xyz[3 * i + 1] > max_[1])
                max_[1] = xyz[3 * i + 1];
            if (xyz[3 * i + 2] < min_[2])
                min_[2] = xyz[3 * i + 2];
            else if (xyz[3 * i + 2] > max_[2])
                max_[2] = xyz[3 * i + 2];
        }

        empty_ = false;
    }
    else
    {
        clear();
    }
}

template <class T> inline void Box<T>::extend(T x, T y, T z)
{
    if (empty_)
    {
        min_[0] = x;
        min_[1] = y;
        min_[2] = z;
        max_[0] = x;
        max_[1] = y;
        max_[2] = z;
        empty_ = false;
    }
    else
    {
        if (x < min_[0])
            min_[0] = x;
        else if (x > max_[0])
            max_[0] = x;
        if (y < min_[1])
            min_[1] = y;
        else if (y > max_[1])
            max_[1] = y;
        if (z < min_[2])
            min_[2] = z;
        else if (z > max_[2])
            max_[2] = z;
    }
}

template <class T> inline void Box<T>::set(const Box<T> &box)
{
    min_[0] = box.min_[0];
    max_[0] = box.max_[0];
    min_[1] = box.min_[1];
    max_[1] = box.max_[1];
    min_[2] = box.min_[2];
    max_[2] = box.max_[2];

    validate();
}

template <class T>
inline void Box<T>::setPercent(const Box<T> &box,
                               const Box<T> &a,
                               const Box<T> &b)
{
    for (int i = 0; i < 3; i++)
    {
        min_[i] = box.min_[i];
        max_[i] = box.max_[i];

        T lengthSrc = a.max_[i] - a.min_[i];

        if (lengthSrc > 0)
        {
            T lengthDst = box.max_[i] - box.min_[i];

            min_[i] += lengthDst * ((b.min_[i] - a.min_[i]) / lengthSrc);
            max_[i] -= lengthDst * ((a.max_[i] - b.max_[i]) / lengthSrc);
        }
    }

    validate();
}

template <class T> inline void Box<T>::translate(const Vector3<T> &v)
{
    for (size_t i = 0; i < 3; i++)
    {
        min_[i] += v[i];
        max_[i] += v[i];
    }

    validate();
}

template <class T> inline void Box<T>::extend(const Box<T> &box)
{
    if (!box.empty())
    {
        if (empty())
        {
            *this = box;
        }
        else
        {
            if (box.min_[0] < min_[0])
                min_[0] = box.min_[0];
            if (box.max_[0] > max_[0])
                max_[0] = box.max_[0];
            if (box.min_[1] < min_[1])
                min_[1] = box.min_[1];
            if (box.max_[1] > max_[1])
                max_[1] = box.max_[1];
            if (box.min_[2] < min_[2])
                min_[2] = box.min_[2];
            if (box.max_[2] > max_[2])
                max_[2] = box.max_[2];

            validate();
        }
    }
}

template <class T> inline void Box<T>::clear()
{
    min_[0] = min_[1] = min_[2] = 0;
    max_[0] = max_[1] = max_[2] = 0;
    empty_ = true;
}

template <class T> inline void Box<T>::validate()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    if ((max_[0] - min_[0] > e) || (max_[1] - min_[1] > e) ||
        (max_[2] - min_[2] > e))
    {
        empty_ = false; // Line/Plane/Cuboid
    }
    else
    {
        empty_ = true;
    }
}

template <class T> inline T Box<T>::maximumLength() const
{
    T max = length(0);

    if (length(1) > max)
    {
        max = length(1);
    }

    if (length(2) > max)
    {
        max = length(2);
    }

    return max;
}

template <class T> inline void Box<T>::getCenter(T &x, T &y, T &z) const
{
    x = min_[0] + ((max_[0] - min_[0]) / 2);
    y = min_[1] + ((max_[1] - min_[1]) / 2);
    z = min_[2] + ((max_[2] - min_[2]) / 2);
}

template <class T> inline Vector3<T> Box<T>::getCenter() const
{
    return Vector3<T>(min_[0] + ((max_[0] - min_[0]) / 2),
                      min_[1] + ((max_[1] - min_[1]) / 2),
                      min_[2] + ((max_[2] - min_[2]) / 2));
}

template <class T> inline T Box<T>::distance(T x, T y, T z) const
{
    T u = 0;
    T v = 0;
    T w = 0;
    T ret;

    getCenter(u, v, w);
    ret = std::sqrt((u - x) * (u - x) + (v - y) * (v - y) + (w - z) * (w - z));

    return ret;
}

template <class T> inline T Box<T>::radius() const
{
    T u, v, w;
    T ret;

    u = (max_[0] - min_[0]) * 0.5;
    v = (max_[1] - min_[1]) * 0.5;
    w = (max_[2] - min_[2]) * 0.5;
    ret = std::sqrt((u * u) + (v * v) + (w * w));

    return ret;
}

template <class T> inline bool Box<T>::intersects(const Box &box) const
{
    return !((min_[0] > box.max_[0] || max_[0] < box.min_[0]) ||
             (min_[1] > box.max_[1] || max_[1] < box.min_[1]) ||
             (min_[2] > box.max_[2] || max_[2] < box.min_[2]));
}

template <class T> inline bool Box<T>::isInside(const Box &box) const
{
    return !((max_[0] > box.max_[0] || min_[0] < box.min_[0]) ||
             (max_[1] > box.max_[1] || min_[1] < box.min_[1]) ||
             (max_[2] > box.max_[2] || min_[2] < box.min_[2]));
}

template <class T> inline bool Box<T>::isInside(T x, T y, T z) const
{
    return !((x < min_[0] || x > max_[0]) || (y < min_[1] || y > max_[1]) ||
             (z < min_[2] || z > max_[2]));
}

template <class T> inline void Box<T>::read(const Json &in)
{
    min_[0] = in["min"][0].number();
    min_[1] = in["min"][1].number();
    min_[2] = in["min"][2].number();
    max_[0] = in["max"][0].number();
    max_[1] = in["max"][1].number();
    max_[2] = in["max"][2].number();
}

template <class T> inline Json &Box<T>::write(Json &out) const
{
    out["min"][0] = min_[0];
    out["min"][1] = min_[1];
    out["min"][2] = min_[2];
    out["max"][0] = max_[0];
    out["max"][1] = max_[1];
    out["max"][2] = max_[2];

    return out;
}

template <class T> std::ostream &operator<<(std::ostream &os, const Box<T> &obj)
{
    return os << std::fixed << "{{" << obj.min(0) << ", " << obj.min(1) << ", "
              << obj.min(2)
              << "}, "
                 "{"
              << obj.max(0) << ", " << obj.max(1) << ", " << obj.max(2) << "}"
              << "}" << std::defaultfloat;
}

#include <WarningsEnable.hpp>

#endif /* BOX_HPP */
