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

/** @file Sphere.hpp */

#ifndef SPHERE_HPP
#define SPHERE_HPP

// Include 3D Forest.
#include <Box.hpp>
#include <Util.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Sphere. */
template <class T> class Sphere
{
public:
    Sphere();
    ~Sphere();

    template <class B> Sphere(B x, B y, B z, B radius);

    void set(T x, T y, T z, T radius);
    void clear();
    bool empty() const { return box_.empty(); }

    const Box<T> &box() const { return box_; }

    bool contains(T x, T y, T z) const;

protected:
    T x_;
    T y_;
    T z_;
    T radius_;
    Box<T> box_;

    void validate();
};

template <class T> inline Sphere<T>::Sphere()
{
    clear();
}

template <class T> inline Sphere<T>::~Sphere()
{
}

template <class T>
template <class B>
inline Sphere<T>::Sphere(B x, B y, B z, B radius)
{
    set(x, y, z, radius);
}

template <class T> inline void Sphere<T>::set(T x, T y, T z, T radius)
{
    x_ = x;
    y_ = y;
    z_ = z;
    radius_ = radius;

    validate();
}

template <class T> inline void Sphere<T>::clear()
{
    x_ = 0;
    y_ = 0;
    z_ = 0;
    radius_ = 0;

    box_.clear();
}

template <class T> inline void Sphere<T>::validate()
{
    box_.set(x_ - radius_,
             y_ - radius_,
             z_ - radius_,
             x_ + radius_,
             y_ + radius_,
             z_ + radius_);
}

template <class T> inline bool Sphere<T>::contains(T x, T y, T z) const
{
    if (box_.contains(x, y, z))
    {
        T distance = std::sqrt(((x_ - x) * (x_ - x)) + ((y_ - y) * (y_ - y)) +
                               ((z_ - z) * (z_ - z)));

        return !(distance > radius_);
    }

    return false;
}

#include <WarningsEnable.hpp>

#endif /* SPHERE_HPP */
