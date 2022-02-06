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

/** @file Cone.hpp */

#ifndef CONE_HPP
#define CONE_HPP

#include <Box.hpp>
#include <Log.hpp>

/** Cone. */
template <class T> class Cone
{
public:
    Cone();
    ~Cone();

    void set(T x, T y, T z1, T z2, T angle);
    void clear();
    bool empty() const { return box_.empty(); }
    const Box<T> &box() const { return box_; }

    bool isInside(T x, T y, T z) const;

protected:
    T x_;
    T y_;
    T z_;
    T z2_;
    T radius_;
    T angle_;
    Box<T> box_;

    void validate();
};

template <class T> inline Cone<T>::Cone()
{
    clear();
}

template <class T> inline Cone<T>::~Cone()
{
}

template <class T> inline void Cone<T>::set(T x, T y, T z1, T z2, T angle)
{
    x_ = x;
    y_ = y;
    z_ = z1;
    z2_ = z2;
    radius_ = 0;
    angle_ = angle;

    validate();
}

template <class T> inline void Cone<T>::clear()
{
    x_ = 0;
    y_ = 0;
    z_ = 0;
    z2_ = 0;
    radius_ = 0;
    angle_ = 0;

    box_.clear();
}

template <class T> inline void Cone<T>::validate()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    T d = std::fabs(z_ - z2_);

    if (d > e)
    {
        radius_ = std::tan(angle_ * 0.01745329) * d;
    }
    else
    {
        radius_ = 0;
    }

    box_.set(x_ - radius_, y_ - radius_, z_, x_ + radius_, y_ + radius_, z2_);
}

template <class T> inline bool Cone<T>::isInside(T x, T y, T z) const
{
    if (box_.isInside(x, y, z))
    {
        // Slow
        T d = std::sqrt(((x_ - x) * (x_ - x)) + ((y_ - y) * (y_ - y)));
        T angle = std::atan(d / std::fabs(z_ - z)) * 57.29578;

        return angle < angle_;
    }

    return false;
}

#endif /* CONE_HPP */
