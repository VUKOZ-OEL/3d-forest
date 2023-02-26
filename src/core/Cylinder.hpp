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

/** @file Cylinder.hpp */

#ifndef CYLINDER_HPP
#define CYLINDER_HPP

#include <Box.hpp>
#include <Geometry.hpp>
#include <Math.hpp>

/** Cylinder. */
template <class T> class Cylinder
{
public:
    Cylinder();
    ~Cylinder();

    template <class B>
    Cylinder(B x, B y, B z, B nx, B ny, B nz, B radius, B length);

    void set(T x, T y, T z, T nx, T ny, T nz, T radius, T length);

    void clear();

    bool empty() const { return box_.empty(); }

    const Box<T> &box() const { return box_; }

    bool isInside(T x, T y, T z) const;

protected:
    T x_;
    T y_;
    T z_;

    T nx_;
    T ny_;
    T nz_;

    T radius_;
    T length_;

    Box<T> box_;

    void validate();
};

template <class T> inline Cylinder<T>::Cylinder()
{
    clear();
}

template <class T> inline Cylinder<T>::~Cylinder()
{
}

template <class T>
template <class B>
inline Cylinder<T>::Cylinder(B x,
                             B y,
                             B z,
                             B nx,
                             B ny,
                             B nz,
                             B radius,
                             B length)
    : x_(static_cast<T>(x)),
      y_(static_cast<T>(y)),
      z_(static_cast<T>(z)),
      nx_(static_cast<T>(nx)),
      ny_(static_cast<T>(ny)),
      nz_(static_cast<T>(nz)),
      radius_(static_cast<T>(radius)),
      length_(static_cast<T>(length))
{
    validate();
}

template <class T>
inline void Cylinder<T>::set(T x,
                             T y,
                             T z,
                             T nx,
                             T ny,
                             T nz,
                             T radius,
                             T length)
{
    x_ = x;
    y_ = y;
    z_ = z;

    nx_ = nx;
    ny_ = ny;
    nz_ = nz;

    radius_ = radius;
    length_ = length;

    validate();
}

template <class T> inline void Cylinder<T>::clear()
{
    x_ = 0;
    y_ = 0;
    z_ = 0;

    nx_ = 0;
    ny_ = 0;
    nz_ = 0;

    radius_ = 0;
    length_ = 0;

    box_.clear();
}

template <class T> inline void Cylinder<T>::validate()
{
    Vector3<T> a(x_, y_, z_);
    Vector3<T> b(x_ + nx_ * length_, y_ + ny_ * length_, z_ + nz_ * length_);
    Vector3<T> c(radius_, radius_, radius_);

    box_.set(Vector3<T>::min(a, b) - c, Vector3<T>::max(a, b) + c);
}

template <class T> inline bool Cylinder<T>::isInside(T x, T y, T z) const
{
    if (!box_.isInside(x, y, z))
    {
        return false;
    }

    T dp = pointPlaneDistance(x, y, z, x_, y_, z_, nx_, ny_, nz_);
    if (dp < 0 || dp > length_)
    {
        return false;
    }

    T dl = pointLineDistance(x, y, z, x_, y_, z_, nx_, ny_, nz_);
    if (dl > radius_)
    {
        return false;
    }

    return true;
}

#endif /* CYLINDER_HPP */
