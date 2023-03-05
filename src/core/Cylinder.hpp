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

    void set(T ax, T ay, T az, T bx, T by, T bz, T radius);
    void set(T x, T y, T z, T nx, T ny, T nz, T radius, T length);

    const Vector3<T> &a() const { return a_; }
    const Vector3<T> &b() const { return b_; }
    const Vector3<T> &n() const { return n_; }
    T radius() const { return radius_; }
    T length() const { return length_; }
    const Box<T> &box() const { return box_; }

    void clear();
    bool empty() const { return box_.empty(); }
    bool isInside(T x, T y, T z) const;

protected:
    Vector3<T> a_;
    Vector3<T> b_;
    Vector3<T> n_;

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
    : a_(static_cast<T>(x), static_cast<T>(y), static_cast<T>(z)),
      b_(static_cast<T>(x + nx * length),
         static_cast<T>(y + ny * length),
         static_cast<T>(z + nz * length)),
      n_(static_cast<T>(nx), static_cast<T>(ny), static_cast<T>(nz)),
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
    a_.set(x, y, z);
    b_.set(x + nx * length, y + ny * length, z + nz * length);
    n_.set(nx, ny, nz);

    radius_ = radius;
    length_ = length;

    validate();
}

template <class T>
inline void Cylinder<T>::set(T ax, T ay, T az, T bx, T by, T bz, T radius)
{
    a_.set(ax, ay, az);
    b_.set(bx, by, bz);
    n_.set(bx - ax, by - ay, bz - az);

    radius_ = radius;
    length_ = n_.length();

    n_.normalize();

    validate();
}

template <class T> inline void Cylinder<T>::clear()
{
    a_.clear();
    b_.clear();
    n_.clear();

    radius_ = 0;
    length_ = 0;

    box_.clear();
}

template <class T> inline void Cylinder<T>::validate()
{
    Vector3<T> c(radius_, radius_, radius_);

    box_.set(Vector3<T>::min(a_, b_) - c, Vector3<T>::max(a_, b_) + c);
}

template <class T> inline bool Cylinder<T>::isInside(T x, T y, T z) const
{
    if (!box_.isInside(x, y, z))
    {
        return false;
    }

    T dp;
    dp = pointPlaneDistance(x, y, z, a_[0], a_[1], a_[2], n_[0], n_[1], n_[2]);
    if (dp < 0 || dp > length_)
    {
        return false;
    }

    T dl;
    dl = pointLineDistance(x, y, z, a_[0], a_[1], a_[2], n_[0], n_[1], n_[2]);
    if (dl > radius_)
    {
        return false;
    }

    return true;
}

#endif /* CYLINDER_HPP */
