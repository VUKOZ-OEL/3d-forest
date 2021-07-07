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

/** @file Vector3.hpp */

#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <Json.hpp>
#include <array>
#include <cmath>

/** Vector 3D. */
template <class T> class Vector3 : public std::array<T, 3>
{
public:
    Vector3();
    Vector3(T v0, T v1, T v2);
    Vector3(uint32_t v24);
    template <class B> Vector3(const Vector3<B> &v);
    template <class B> Vector3<T> &operator=(const Vector3<B> &v);

    template <class B> void set(B v0, B v1, B v2);

    void clear();

    T length() const;
    T min() const;
    T max() const;

    void normalize();
    Vector3<T> normalized() const;

    void read(const Json &in);
    Json &write(Json &out) const;

    Vector3<T> crossProduct(const Vector3<T> &v) const;
    static Vector3<T> crossProduct(const Vector3<T> &a, const Vector3<T> &b)
    {
        return a.crossProduct(b);
    }

    Vector3<T> rotated(const Vector3<T> &axis, double angle) const;

    friend Vector3<T> operator+(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
    }

    friend Vector3<T> operator-(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
    }

    friend Vector3<T> operator*(const Vector3<T> &a, T b)
    {
        return Vector3<T>(a[0] * b, a[1] * b, a[2] * b);
    }

    friend Vector3<T> operator*(T a, const Vector3<T> &b)
    {
        return Vector3<T>(a * b[0], a * b[1], a * b[2]);
    }

    friend Vector3<T> operator*(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
    }

    friend Vector3<T> operator/(const Vector3<T> &a, T b)
    {
        return Vector3<T>(a[0] / b, a[1] / b, a[2] / b);
    }

    friend Vector3<T> operator/(T a, const Vector3<T> &b)
    {
        return Vector3<T>(a / b[0], a / b[1], a / b[2]);
    }

    friend Vector3<T> operator/(const Vector3<T> &a, const Vector3<T> &b)
    {
        return Vector3<T>(a[0] / b[0], a[1] / b[1], a[2] / b[2]);
    }
};

template <class T> inline Vector3<T>::Vector3()
{
    clear();
}

template <class T> inline Vector3<T>::Vector3(T v0, T v1, T v2)
{
    this->operator[](0) = v0;
    this->operator[](1) = v1;
    this->operator[](2) = v2;
}

template <class T> inline Vector3<T>::Vector3(uint32_t v24)
{
    this->operator[](0) = static_cast<T>((v24 & 0xff0000) >> 16) / 255;
    this->operator[](1) = static_cast<T>((v24 & 0x00ff00) >> 8) / 255;
    this->operator[](2) = static_cast<T>((v24 & 0x0000ff)) / 255;
}

template <class T>
template <class B>
inline Vector3<T> &Vector3<T>::operator=(const Vector3<B> &v)
{
    this->operator[](0) = static_cast<T>(v[0]);
    this->operator[](1) = static_cast<T>(v[1]);
    this->operator[](2) = static_cast<T>(v[2]);
    return *this;
}

template <class T>
template <class B>
inline void Vector3<T>::set(B v0, B v1, B v2)
{
    this->operator[](0) = static_cast<T>(v0);
    this->operator[](1) = static_cast<T>(v1);
    this->operator[](2) = static_cast<T>(v2);
}

template <class T> inline void Vector3<T>::clear()
{
    this->operator[](0) = 0;
    this->operator[](1) = 0;
    this->operator[](2) = 0;
}

template <class T> inline T Vector3<T>::min() const
{
    return this->operator[](0) < this->operator[](1)
               ? (this->operator[](0) < this->operator[](2)
                      ? this->operator[](0)
                      : this->operator[](2))
               : (this->operator[](1) < this->operator[](2)
                      ? this->operator[](1)
                      : this->operator[](2));
}

template <class T> inline T Vector3<T>::max() const
{
    return this->operator[](0) > this->operator[](1)
               ? (this->operator[](0) > this->operator[](2)
                      ? this->operator[](0)
                      : this->operator[](2))
               : (this->operator[](1) > this->operator[](2)
                      ? this->operator[](1)
                      : this->operator[](2));
}

template <class T> inline T Vector3<T>::length() const
{
    return static_cast<T>(
        std::sqrt((this->operator[](0) * this->operator[](0)) +
                  (this->operator[](1) * this->operator[](1)) +
                  (this->operator[](2) * this->operator[](2))));
}

template <class T> inline void Vector3<T>::normalize()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    T len = length();

    if (len > e)
    {
        T s = 1 / len;
        this->operator[](0) *= s;
        this->operator[](1) *= s;
        this->operator[](2) *= s;
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
    return Vector3<T>(this->operator[](1) * v[2] - this->operator[](2) * v[1],
                      this->operator[](2) * v[0] - this->operator[](0) * v[2],
                      this->operator[](0) * v[1] - this->operator[](1) * v[0]);
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

    return Vector3<T>(this->operator[](0) * (w2 + x2 - y2 - z2) +
                          this->operator[](1) * 2 * (xr * yr + w * zr) +
                          this->operator[](2) * 2 * (xr * zr - w * yr),
                      this->operator[](0) * 2 * (xr * yr - w * zr) +
                          this->operator[](1) * (w2 - x2 + y2 - z2) +
                          this->operator[](2) * 2 * (yr * zr + w * xr),
                      this->operator[](0) * 2 * (xr * zr + w * yr) +
                          this->operator[](1) * 2 * (yr * zr - w * xr) +
                          this->operator[](2) * (w2 - x2 - y2 + z2));
}

template <class T> inline void Vector3<T>::read(const Json &in)
{
    this->operator[](0) = static_cast<T>(in[0].number());
    this->operator[](1) = static_cast<T>(in[1].number());
    this->operator[](2) = static_cast<T>(in[2].number());
}

template <class T> inline Json &Vector3<T>::write(Json &out) const
{
    out[0] = this->operator[](0);
    out[1] = this->operator[](1);
    out[2] = this->operator[](2);

    return out;
}

template <class T>
std::ostream &operator<<(std::ostream &os, const Vector3<T> &obj)
{
    return os << std::fixed << "{" << obj[0] << ", " << obj[1] << ", " << obj[2]
              << "}" << std::defaultfloat;
}

#endif /* VECTOR3_HPP */