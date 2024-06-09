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

/** @file Vector2.hpp */

#ifndef VECTOR2_HPP
#define VECTOR2_HPP

// Include std.
#include <array>
#include <cmath>

// Include 3D Forest.
#include <Json.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Vector 2D. */
template <class T> class Vector2 : public std::array<T, 2>
{
public:
    Vector2();
    Vector2(T v0, T v1);
    Vector2(uint32_t v16);
    template <class B> Vector2(const Vector2<B> &v);
    template <class B> Vector2<T> &operator=(const Vector2<B> &v);

    template <class B> void set(B v0, B v1);

    void clear();

    T length() const;
    T min() const;
    T max() const;

    void normalize();
    Vector2<T> normalized() const;

    friend Vector2<T> operator+(const Vector2<T> &a, const Vector2<T> &b)
    {
        return Vector2<T>(a[0] + b[0], a[1] + b[1]);
    }

    friend Vector2<T> operator-(const Vector2<T> &a, const Vector2<T> &b)
    {
        return Vector2<T>(a[0] - b[0], a[1] - b[1]);
    }

    friend Vector2<T> operator*(const Vector2<T> &a, T b)
    {
        return Vector2<T>(a[0] * b, a[1] * b);
    }

    friend Vector2<T> operator*(T a, const Vector2<T> &b)
    {
        return Vector2<T>(a * b[0], a * b[1]);
    }

    friend Vector2<T> operator*(const Vector2<T> &a, const Vector2<T> &b)
    {
        return Vector2<T>(a[0] * b[0], a[1] * b[1]);
    }

    friend Vector2<T> operator/(const Vector2<T> &a, T b)
    {
        return Vector2<T>(a[0] / b, a[1] / b);
    }

    friend Vector2<T> operator/(T a, const Vector2<T> &b)
    {
        return Vector2<T>(a / b[0], a / b[1]);
    }

    friend Vector2<T> operator/(const Vector2<T> &a, const Vector2<T> &b)
    {
        return Vector2<T>(a[0] / b[0], a[1] / b[1]);
    }
};

template <class T> inline Vector2<T>::Vector2()
{
    clear();
}

template <class T> inline Vector2<T>::Vector2(T v0, T v1)
{
    this->operator[](0) = v0;
    this->operator[](1) = v1;
}

template <class T> inline Vector2<T>::Vector2(uint32_t v16)
{
    this->operator[](0) = static_cast<T>((v16 & 0x00ff00U) >> 8) / 255;
    this->operator[](1) = static_cast<T>((v16 & 0x0000ffU)) / 255;
}

template <class T>
template <class B>
inline Vector2<T> &Vector2<T>::operator=(const Vector2<B> &v)
{
    this->operator[](0) = static_cast<T>(v[0]);
    this->operator[](1) = static_cast<T>(v[1]);
    return *this;
}

template <class T> template <class B> inline void Vector2<T>::set(B v0, B v1)
{
    this->operator[](0) = static_cast<T>(v0);
    this->operator[](1) = static_cast<T>(v1);
}

template <class T> inline void Vector2<T>::clear()
{
    this->operator[](0) = 0;
    this->operator[](1) = 0;
}

template <class T> inline T Vector2<T>::min() const
{
    return this->operator[](0) < this->operator[](1) ? this->operator[](0)
                                                     : this->operator[](1);
}

template <class T> inline T Vector2<T>::max() const
{
    return this->operator[](0) > this->operator[](1) ? this->operator[](0)
                                                     : this->operator[](1);
}

template <class T> inline T Vector2<T>::length() const
{
    return static_cast<T>(
        std::sqrt((this->operator[](0) * this->operator[](0)) +
                  (this->operator[](1) * this->operator[](1))));
}

template <class T> inline void Vector2<T>::normalize()
{
    constexpr T e = std::numeric_limits<T>::epsilon();
    T len = length();

    if (len > e)
    {
        T s = 1 / len;
        this->operator[](0) *= s;
        this->operator[](1) *= s;
    }
}

template <class T> inline Vector2<T> Vector2<T>::normalized() const
{
    Vector2<T> v(*this);
    v.normalize();
    return v;
}

template <class T> inline void fromJson(Vector2<T> &out, const Json &in)
{
    fromJson(out(0), in[0]);
    fromJson(out(1), in[1]);
}

template <class T> inline void toJson(Json &out, const Vector2<T> &in)
{
    toJson(out[0], in(0));
    toJson(out[1], in(1));
}

template <class T> inline std::string toString(const Vector2<T> &in)
{
    return "(" + std::to_string(in[0]) + ", " + std::to_string(in[1]) + ")";
}

template <class T>
std::ostream &operator<<(std::ostream &out, const Vector2<T> &in)
{
    return out << toString(in);
}

#include <WarningsEnable.hpp>

#endif /* VECTOR2_HPP */