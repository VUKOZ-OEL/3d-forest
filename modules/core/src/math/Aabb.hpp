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
    @file Aabb.hpp
*/

#ifndef AABB_HPP
#define AABB_HPP

#include <Json.hpp>
#include <cmath>
#include <limits>
#include <vector>

/** Axis-Aligned Bounding Box. */
template <class T> class Aabb
{
public:
    Aabb();
    ~Aabb();

    template <class B> Aabb(const Aabb<B> &box);

    void set(T x1, T y1, T z1, T x2, T y2, T z2);
    void set(const std::vector<T> &xyz);
    void set(const Aabb<T> &box);
    void setPercent(const Aabb<T> &box, const Aabb<T> &a, const Aabb<T> &b);

    void extend(const Aabb<T> &box);
    void clear();

    bool empty() const { return empty_; }

    const T &min(size_t idx) const { return min_[idx]; }
    const T &max(size_t idx) const { return max_[idx]; }

    void getCenter(T &x, T &y, T &z) const;
    T distance(T x, T y, T z) const;
    T radius() const;

    bool intersects(const Aabb<T> &box) const;
    bool isInside(const Aabb<T> &box) const;
    bool isInside(T x, T y, T z) const;

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    T min_[3];
    T max_[3];
    bool empty_;

    void validate();
};

template <class T> inline Aabb<T>::Aabb()
{
    clear();
}

template <class T> inline Aabb<T>::~Aabb()
{
}

template <class T> template <class B> inline Aabb<T>::Aabb(const Aabb<B> &box)
{
    min_[0] = static_cast<T>(box.min_[0]);
    min_[1] = static_cast<T>(box.min_[1]);
    min_[2] = static_cast<T>(box.min_[2]);
    max_[0] = static_cast<T>(box.max_[0]);
    max_[1] = static_cast<T>(box.max_[1]);
    max_[2] = static_cast<T>(box.max_[2]);
    validate();
}

template <class T> inline void Aabb<T>::set(T x1, T y1, T z1, T x2, T y2, T z2)
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

template <class T> inline void Aabb<T>::set(const std::vector<T> &xyz)
{
    size_t n = xyz.size() / 3;
    if (n > 0)
    {
        T min_x = xyz[0];
        T min_y = xyz[1];
        T min_z = xyz[2];
        T max_x = xyz[0];
        T max_y = xyz[1];
        T max_z = xyz[2];

        for (size_t i = 1; i < n; i++)
        {
            if (xyz[3 * i + 0] < min_x)
                min_x = xyz[3 * i + 0];
            else if (xyz[3 * i + 0] > max_x)
                max_x = xyz[3 * i + 0];
            if (xyz[3 * i + 1] < min_y)
                min_y = xyz[3 * i + 1];
            else if (xyz[3 * i + 1] > max_y)
                max_y = xyz[3 * i + 1];
            if (xyz[3 * i + 2] < min_z)
                min_z = xyz[3 * i + 2];
            else if (xyz[3 * i + 2] > max_z)
                max_z = xyz[3 * i + 2];
        }

        set(min_x, min_y, min_z, max_x, max_y, max_z);
    }
    else
    {
        clear();
    }
}

template <class T> inline void Aabb<T>::set(const Aabb<T> &box)
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
inline void Aabb<T>::setPercent(const Aabb<T> &box,
                                const Aabb<T> &a,
                                const Aabb<T> &b)
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

template <class T> inline void Aabb<T>::extend(const Aabb<T> &box)
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

template <class T> inline void Aabb<T>::clear()
{
    min_[0] = min_[1] = min_[2] = 0;
    max_[0] = max_[1] = max_[2] = 0;
    empty_ = true;
}

template <class T> inline void Aabb<T>::validate()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    if ((max_[0] - min_[0] > e) && (max_[1] - min_[1] > e) &&
        (max_[2] - min_[2] > e))
    {
        empty_ = false;
    }
    else
    {
        empty_ = true;
    }
}

template <class T> inline void Aabb<T>::getCenter(T &x, T &y, T &z) const
{
    x = min_[0] + ((max_[0] - min_[0]) / 2);
    y = min_[1] + ((max_[1] - min_[1]) / 2);
    z = min_[2] + ((max_[2] - min_[2]) / 2);
}

template <class T> inline T Aabb<T>::distance(T x, T y, T z) const
{
    T u, v, w;
    T ret;

    getCenter(u, v, w);
    ret = std::sqrt((u - x) * (u - x) + (v - y) * (v - y) + (w - z) * (w - z));

    return ret;
}

template <class T> inline T Aabb<T>::radius() const
{
    T u, v, w;
    T ret;

    u = (max_[0] - min_[0]) * 0.5;
    v = (max_[1] - min_[1]) * 0.5;
    w = (max_[2] - min_[2]) * 0.5;
    ret = std::sqrt((u * u) + (v * v) + (w * w));

    return ret;
}

template <class T> inline bool Aabb<T>::intersects(const Aabb &box) const
{
    return !((min_[0] > box.max_[0] || max_[0] < box.min_[0]) ||
             (min_[1] > box.max_[1] || max_[1] < box.min_[1]) ||
             (min_[2] > box.max_[2] || max_[2] < box.min_[2]));
}

template <class T> inline bool Aabb<T>::isInside(const Aabb &box) const
{
    return !((max_[0] > box.max_[0] || min_[0] < box.min_[0]) ||
             (max_[1] > box.max_[1] || min_[1] < box.min_[1]) ||
             (max_[2] > box.max_[2] || min_[2] < box.min_[2]));
}

template <class T> inline bool Aabb<T>::isInside(T x, T y, T z) const
{
    return !((x < min_[0] || x > max_[0]) || (y < min_[1] || y > max_[1]) ||
             (z < min_[2] || z > max_[2]));
}

template <class T> inline void Aabb<T>::read(const Json &in)
{
    min_[0] = in["min"][0].number();
    min_[1] = in["min"][1].number();
    min_[2] = in["min"][2].number();
    max_[0] = in["max"][0].number();
    max_[1] = in["max"][1].number();
    max_[2] = in["max"][2].number();
}

template <class T> inline Json &Aabb<T>::write(Json &out) const
{
    out["min"][0] = min_[0];
    out["min"][1] = min_[1];
    out["min"][2] = min_[2];
    out["max"][0] = max_[0];
    out["max"][1] = max_[1];
    out["max"][2] = max_[2];

    return out;
}

#endif /* AABB_HPP */
