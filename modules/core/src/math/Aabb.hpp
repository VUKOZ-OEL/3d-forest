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

/** Axis-Aligned Bounding Box. */
template <class T> class Aabb
{
public:
    Aabb();
    ~Aabb();

    template <class B> Aabb(const Aabb<B> &box);

    void set(T x1, T y1, T z1, T x2, T y2, T z2);
    void clear();

    const T &min(size_t idx) const { return min_[idx]; }
    const T &max(size_t idx) const { return max_[idx]; }

    void getCenter(T &x, T &y, T &z) const;
    bool intersects(const Aabb<T> &box) const;
    bool isInside(const Aabb<T> &box) const;

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    T min_[3];
    T max_[3];
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
}

template <class T> inline void Aabb<T>::clear()
{
    min_[0] = min_[1] = min_[2] = 0;
    max_[0] = max_[1] = max_[2] = 0;
}

template <class T> inline void Aabb<T>::getCenter(T &x, T &y, T &z) const
{
    x = min_[0] + ((max_[0] - min_[0]) / 2);
    y = min_[1] + ((max_[1] - min_[1]) / 2);
    z = min_[2] + ((max_[2] - min_[2]) / 2);
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
