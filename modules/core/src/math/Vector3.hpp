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
    @file Vector3.hpp
*/

#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>
#include <limits>

/** Vector 3D. */
template <class T> class Vector3
{
public:
    Vector3();
    Vector3(const Vector3<T> &v) = default;
    Vector3(T x, T y, T z);
    template <class B> Vector3(const Vector3<B> &v);
    ~Vector3();
    Vector3<T> &operator=(const Vector3<T> &v) = default;

    const T &operator()(size_t idx) const { return data_[idx]; }
    T &operator()(size_t idx) { return data_[idx]; }
    const T &operator[](size_t idx) const { return data_[idx]; }
    T &operator[](size_t idx) { return data_[idx]; }
    const T *data() const { return &data_[0]; };
    T x() const { return data_[0]; };
    T y() const { return data_[1]; };
    T z() const { return data_[2]; };

    T length() const;
    void normalize();
    Vector3<T> normalized() const;
    Vector3<T> crossProduct(const Vector3<T> &v) const;
    static Vector3<T> crossProduct(const Vector3<T> &a, const Vector3<T> &b)
    {
        return a.crossProduct(b);
    }

    Vector3<T> rotated(const Vector3<T> &axis, double angle) const;

    friend Vector3<T> operator+(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a(0) + b(0), a(1) + b(1), a(2) + b(2));
    }

    friend Vector3<T> operator-(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a(0) - b(0), a(1) - b(1), a(2) - b(2));
    }

    friend Vector3<T> operator*(const Vector3<T> &a, T b)
    {
        return Vector3<T>(a(0) * b, a(1) * b, a(2) * b);
    }

    friend Vector3<T> operator*(T a, const Vector3<T> &b)
    {
        return Vector3<T>(a * b(0), a * b(1), a * b(2));
    }

    friend Vector3<T> operator*(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a(0) * b(0), a(1) * b(1), a(2) * b(2));
    }

    friend Vector3<T> operator/(const Vector3<T> &a, T b)
    {
        return Vector3<T>(a(0) / b, a(1) / b, a(2) / b);
    }

    friend Vector3<T> operator/(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a(0) / b(0), a(1) / b(1), a(2) / b(2));
    }

protected:
    T data_[3];
};

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

template <class T> inline Vector3<T>::Vector3()
{
    data_[0] = data_[1] = data_[2] = 0;
}

template <class T> inline Vector3<T>::Vector3(T x, T y, T z)
{
    data_[0] = x;
    data_[1] = y;
    data_[2] = z;
}

template <class T>
template <class B>
inline Vector3<T>::Vector3(const Vector3<B> &v)
{
    data_[0] = static_cast<T>(v(0));
    data_[1] = static_cast<T>(v(1));
    data_[2] = static_cast<T>(v(2));
}

template <class T> inline Vector3<T>::~Vector3()
{
    // empty
}

template <class T> inline T Vector3<T>::length() const
{
    return static_cast<T>(std::sqrt(
        (data_[0] * data_[0]) + (data_[1] * data_[1]) + (data_[2] * data_[2])));
}

template <class T> inline void Vector3<T>::normalize()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    T len = length();

    if (len > e)
    {
        T s = 1 / len;
        data_[0] *= s;
        data_[1] *= s;
        data_[2] *= s;
    }
}

template <class T> inline Vector3<T> Vector3<T>::normalized() const
{
    Vector3<T> v(*this);
    v.normalize();
    return v;
}

template <class T>
inline Vector3<T> Vector3<T>::crossProduct(const Vector3<T> &v) const
{
    return Vector3<T>(data_[1] * v(2) - data_[2] * v(1),
                      data_[2] * v(0) - data_[0] * v(2),
                      data_[0] * v(1) - data_[1] * v(0));
}

template <class T>
inline Vector3<T> Vector3<T>::rotated(const Vector3<T> &axis,
                                      double angle) const
{
    Vector3<T> n = axis.normalized();
    T w = static_cast<T>(std::cos(-angle / 2.0));
    T s = static_cast<T>(std::sin(-angle / 2.0));
    T xr = n[0] * s;
    T yr = n[1] * s;
    T zr = n[2] * s;
    T w2 = w * w;
    T x2 = xr * xr;
    T y2 = yr * yr;
    T z2 = zr * zr;

    return Vector3<T>(
        data_[0] * (w2 + x2 - y2 - z2) + data_[1] * 2 * (xr * yr + w * zr) +
            data_[2] * 2 * (xr * zr - w * yr),
        data_[0] * 2 * (xr * yr - w * zr) + data_[1] * (w2 - x2 + y2 - z2) +
            data_[2] * 2 * (yr * zr + w * xr),
        data_[0] * 2 * (xr * zr + w * yr) + data_[1] * 2 * (yr * zr - w * xr) +
            data_[2] * (w2 - x2 - y2 + z2));
}

#endif /* VECTOR3_HPP */