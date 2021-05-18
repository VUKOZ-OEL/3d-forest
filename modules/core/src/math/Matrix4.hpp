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
    @file Matrix4.hpp
*/

#ifndef MATRIX4_HPP
#define MATRIX4_HPP

#include <Vector3.hpp>

/** Matrix 4x4. */
template <class T> class Matrix4
{
public:
    Matrix4();
    Matrix4(const Matrix4<T> &m) = default;
    template <class B> Matrix4(const Matrix4<B> &m);
    template <class B> Matrix4(const B *data);
    Matrix4(T m00,
            T m01,
            T m02,
            T m03,
            T m10,
            T m11,
            T m12,
            T m13,
            T m20,
            T m21,
            T m22,
            T m23,
            T m30,
            T m31,
            T m32,
            T m33);
    ~Matrix4();
    Matrix4<T> &operator=(const Matrix4<T> &m) = default;

    template <class B> void set(const B *data);

    const T &operator()(size_t r, size_t c) const { return data_[c][r]; }
    T &operator()(size_t r, size_t c) { return data_[c][r]; }
    const T *data() const { return &data_[0][0]; }

    void setToIdentity();
    void invert();
    Matrix4<T> inverted() const;

    void translate(T x, T y, T z);

    void perspective(T fovy, T aspect, T near, T far);
    void ortho(T left, T right, T bottom, T top, T near, T far);
    void lookAt(const Vector3<T> &eye,
                const Vector3<T> &center,
                const Vector3<T> &up);

    Vector3<T> map(const Vector3<T> &v);
    Vector3<T> mapVector(const Vector3<T> &v);

    // This is ModelViewProjection matrix
    Vector3<T> project(const Vector3<T> &v);

    // This is inverted ModelViewProjection matrix
    Vector3<T> unproject(const Vector3<T> &v);

    void multiply(T &x, T &y, T &z, T &w);

    friend Matrix4<T> operator*(const Matrix4<T> &a, const Matrix4<T> &b)
    {
        return Matrix4<T>(
            a.data_[0][0] * b.data_[0][0] + a.data_[1][0] * b.data_[0][1] +
                a.data_[2][0] * b.data_[0][2] + a.data_[3][0] * b.data_[0][3],
            a.data_[0][0] * b.data_[1][0] + a.data_[1][0] * b.data_[1][1] +
                a.data_[2][0] * b.data_[1][2] + a.data_[3][0] * b.data_[1][3],
            a.data_[0][0] * b.data_[2][0] + a.data_[1][0] * b.data_[2][1] +
                a.data_[2][0] * b.data_[2][2] + a.data_[3][0] * b.data_[2][3],
            a.data_[0][0] * b.data_[3][0] + a.data_[1][0] * b.data_[3][1] +
                a.data_[2][0] * b.data_[3][2] + a.data_[3][0] * b.data_[3][3],

            a.data_[0][1] * b.data_[0][0] + a.data_[1][1] * b.data_[0][1] +
                a.data_[2][1] * b.data_[0][2] + a.data_[3][1] * b.data_[0][3],
            a.data_[0][1] * b.data_[1][0] + a.data_[1][1] * b.data_[1][1] +
                a.data_[2][1] * b.data_[1][2] + a.data_[3][1] * b.data_[1][3],
            a.data_[0][1] * b.data_[2][0] + a.data_[1][1] * b.data_[2][1] +
                a.data_[2][1] * b.data_[2][2] + a.data_[3][1] * b.data_[2][3],
            a.data_[0][1] * b.data_[3][0] + a.data_[1][1] * b.data_[3][1] +
                a.data_[2][1] * b.data_[3][2] + a.data_[3][1] * b.data_[3][3],

            a.data_[0][2] * b.data_[0][0] + a.data_[1][2] * b.data_[0][1] +
                a.data_[2][2] * b.data_[0][2] + a.data_[3][2] * b.data_[0][3],
            a.data_[0][2] * b.data_[1][0] + a.data_[1][2] * b.data_[1][1] +
                a.data_[2][2] * b.data_[1][2] + a.data_[3][2] * b.data_[1][3],
            a.data_[0][2] * b.data_[2][0] + a.data_[1][2] * b.data_[2][1] +
                a.data_[2][2] * b.data_[2][2] + a.data_[3][2] * b.data_[2][3],
            a.data_[0][2] * b.data_[3][0] + a.data_[1][2] * b.data_[3][1] +
                a.data_[2][2] * b.data_[3][2] + a.data_[3][2] * b.data_[3][3],

            a.data_[0][3] * b.data_[0][0] + a.data_[1][3] * b.data_[0][1] +
                a.data_[2][3] * b.data_[0][2] + a.data_[3][3] * b.data_[0][3],
            a.data_[0][3] * b.data_[1][0] + a.data_[1][3] * b.data_[1][1] +
                a.data_[2][3] * b.data_[1][2] + a.data_[3][3] * b.data_[1][3],
            a.data_[0][3] * b.data_[2][0] + a.data_[1][3] * b.data_[2][1] +
                a.data_[2][3] * b.data_[2][2] + a.data_[3][3] * b.data_[2][3],
            a.data_[0][3] * b.data_[3][0] + a.data_[1][3] * b.data_[3][1] +
                a.data_[2][3] * b.data_[3][2] + a.data_[3][3] * b.data_[3][3]);
    }

protected:
    T data_[4][4];
};

template <class T> inline Matrix4<T>::Matrix4()
{
    setToIdentity();
}

template <class T> inline Matrix4<T>::~Matrix4()
{
}

template <class T>
template <class B>
inline Matrix4<T>::Matrix4(const Matrix4<B> &m)
{
    data_[0][0] = static_cast<T>(m.data_[0][0]);
    data_[0][1] = static_cast<T>(m.data_[0][1]);
    data_[0][2] = static_cast<T>(m.data_[0][2]);
    data_[0][3] = static_cast<T>(m.data_[0][3]);
    data_[1][0] = static_cast<T>(m.data_[1][0]);
    data_[1][1] = static_cast<T>(m.data_[1][1]);
    data_[1][2] = static_cast<T>(m.data_[1][2]);
    data_[1][3] = static_cast<T>(m.data_[1][3]);
    data_[2][0] = static_cast<T>(m.data_[2][0]);
    data_[2][1] = static_cast<T>(m.data_[2][1]);
    data_[2][2] = static_cast<T>(m.data_[2][2]);
    data_[2][3] = static_cast<T>(m.data_[2][3]);
    data_[3][0] = static_cast<T>(m.data_[3][0]);
    data_[3][1] = static_cast<T>(m.data_[3][1]);
    data_[3][2] = static_cast<T>(m.data_[3][2]);
    data_[3][3] = static_cast<T>(m.data_[3][3]);
}

template <class T> template <class B> inline Matrix4<T>::Matrix4(const B *data)
{
    set(data);
}

template <class T> template <class B> inline void Matrix4<T>::set(const B *data)
{
    data_[0][0] = static_cast<T>(data[0]);
    data_[0][1] = static_cast<T>(data[1]);
    data_[0][2] = static_cast<T>(data[2]);
    data_[0][3] = static_cast<T>(data[3]);

    data_[1][0] = static_cast<T>(data[4]);
    data_[1][1] = static_cast<T>(data[5]);
    data_[1][2] = static_cast<T>(data[6]);
    data_[1][3] = static_cast<T>(data[7]);

    data_[2][0] = static_cast<T>(data[8]);
    data_[2][1] = static_cast<T>(data[9]);
    data_[2][2] = static_cast<T>(data[10]);
    data_[2][3] = static_cast<T>(data[11]);

    data_[3][0] = static_cast<T>(data[12]);
    data_[3][1] = static_cast<T>(data[13]);
    data_[3][2] = static_cast<T>(data[14]);
    data_[3][3] = static_cast<T>(data[15]);
}

template <class T>
inline Matrix4<T>::Matrix4(T m00,
                           T m01,
                           T m02,
                           T m03,
                           T m10,
                           T m11,
                           T m12,
                           T m13,
                           T m20,
                           T m21,
                           T m22,
                           T m23,
                           T m30,
                           T m31,
                           T m32,
                           T m33)
{
    data_[0][0] = m00;
    data_[1][0] = m01;
    data_[2][0] = m02;
    data_[3][0] = m03;

    data_[0][1] = m10;
    data_[1][1] = m11;
    data_[2][1] = m12;
    data_[3][1] = m13;

    data_[0][2] = m20;
    data_[1][2] = m21;
    data_[2][2] = m22;
    data_[3][2] = m23;

    data_[0][3] = m30;
    data_[1][3] = m31;
    data_[2][3] = m32;
    data_[3][3] = m33;
}

template <class T> inline void Matrix4<T>::setToIdentity()
{
    data_[0][0] = data_[1][1] = data_[2][2] = data_[3][3] = 1;

    data_[0][1] = data_[0][2] = data_[0][3] = data_[1][0] = data_[1][2] =
        data_[1][3] = data_[2][0] = data_[2][1] = data_[2][3] = data_[3][0] =
            data_[3][1] = data_[3][2] = 0;
}

template <class T> inline void Matrix4<T>::invert()
{
    T v[16];
    for (int i = 0; i < 4; i++)
    {
        v[i * 4 + 0] = data_[i][0];
        v[i * 4 + 1] = data_[i][1];
        v[i * 4 + 2] = data_[i][2];
        v[i * 4 + 3] = data_[i][3];
    }

    T d = (v[2 * 4 + 0] * v[3 * 4 + 1] - v[3 * 4 + 0] * v[2 * 4 + 1]) *
              (v[0 * 4 + 2] * v[1 * 4 + 3] - v[1 * 4 + 2] * v[0 * 4 + 3]) +
          (-1) * (v[2 * 4 + 0] * v[3 * 4 + 2] - v[3 * 4 + 0] * v[2 * 4 + 2]) *
              (v[0 * 4 + 1] * v[1 * 4 + 3] - v[1 * 4 + 1] * v[0 * 4 + 3]) +
          (v[2 * 4 + 1] * v[3 * 4 + 2] - v[3 * 4 + 1] * v[2 * 4 + 2]) *
              (v[0 * 4 + 0] * v[1 * 4 + 3] - v[1 * 4 + 0] * v[0 * 4 + 3]) +
          (-1) * (v[2 * 4 + 1] * v[3 * 4 + 3] - v[3 * 4 + 1] * v[2 * 4 + 3]) *
              (v[0 * 4 + 0] * v[1 * 4 + 2] - v[1 * 4 + 0] * v[0 * 4 + 2]) +
          (v[2 * 4 + 0] * v[3 * 4 + 3] - v[3 * 4 + 0] * v[2 * 4 + 3]) *
              (v[0 * 4 + 1] * v[1 * 4 + 2] - v[1 * 4 + 1] * v[0 * 4 + 2]) +
          (v[2 * 4 + 2] * v[3 * 4 + 3] - v[3 * 4 + 2] * v[2 * 4 + 3]) *
              (v[0 * 4 + 0] * v[1 * 4 + 1] - v[1 * 4 + 0] * v[0 * 4 + 1]);

    constexpr T e = std::numeric_limits<T>::epsilon();
    if (d > e || d < -e)
    {
        data_[0][0] = (v[1 * 4 + 1] * v[2 * 4 + 2] * v[3 * 4 + 3] +
                       v[1 * 4 + 2] * v[2 * 4 + 3] * v[3 * 4 + 1] +
                       v[1 * 4 + 3] * v[2 * 4 + 1] * v[3 * 4 + 2] -
                       v[3 * 4 + 1] * v[2 * 4 + 2] * v[1 * 4 + 3] -
                       v[3 * 4 + 2] * v[2 * 4 + 3] * v[1 * 4 + 1] -
                       v[3 * 4 + 3] * v[2 * 4 + 1] * v[1 * 4 + 2]) /
                      d;
        data_[0][1] = -(v[0 * 4 + 1] * v[2 * 4 + 2] * v[3 * 4 + 3] +
                        v[0 * 4 + 2] * v[2 * 4 + 3] * v[3 * 4 + 1] +
                        v[0 * 4 + 3] * v[2 * 4 + 1] * v[3 * 4 + 2] -
                        v[3 * 4 + 1] * v[2 * 4 + 2] * v[0 * 4 + 3] -
                        v[3 * 4 + 2] * v[2 * 4 + 3] * v[0 * 4 + 1] -
                        v[3 * 4 + 3] * v[2 * 4 + 1] * v[0 * 4 + 2]) /
                      d;
        data_[0][2] = (v[0 * 4 + 1] * v[1 * 4 + 2] * v[3 * 4 + 3] +
                       v[0 * 4 + 2] * v[1 * 4 + 3] * v[3 * 4 + 1] +
                       v[0 * 4 + 3] * v[1 * 4 + 1] * v[3 * 4 + 2] -
                       v[3 * 4 + 1] * v[1 * 4 + 2] * v[0 * 4 + 3] -
                       v[3 * 4 + 2] * v[1 * 4 + 3] * v[0 * 4 + 1] -
                       v[3 * 4 + 3] * v[1 * 4 + 1] * v[0 * 4 + 2]) /
                      d;
        data_[0][3] = -(v[0 * 4 + 1] * v[1 * 4 + 2] * v[2 * 4 + 3] +
                        v[0 * 4 + 2] * v[1 * 4 + 3] * v[2 * 4 + 1] +
                        v[0 * 4 + 3] * v[1 * 4 + 1] * v[2 * 4 + 2] -
                        v[2 * 4 + 1] * v[1 * 4 + 2] * v[0 * 4 + 3] -
                        v[2 * 4 + 2] * v[1 * 4 + 3] * v[0 * 4 + 1] -
                        v[2 * 4 + 3] * v[1 * 4 + 1] * v[0 * 4 + 2]) /
                      d;

        data_[1][0] = -(v[1 * 4 + 0] * v[2 * 4 + 2] * v[3 * 4 + 3] +
                        v[1 * 4 + 2] * v[2 * 4 + 3] * v[3 * 4 + 0] +
                        v[1 * 4 + 3] * v[2 * 4 + 0] * v[3 * 4 + 2] -
                        v[3 * 4 + 0] * v[2 * 4 + 2] * v[1 * 4 + 3] -
                        v[3 * 4 + 2] * v[2 * 4 + 3] * v[1 * 4 + 0] -
                        v[3 * 4 + 3] * v[2 * 4 + 0] * v[1 * 4 + 2]) /
                      d;
        data_[1][1] = (v[0 * 4 + 0] * v[2 * 4 + 2] * v[3 * 4 + 3] +
                       v[0 * 4 + 2] * v[2 * 4 + 3] * v[3 * 4 + 0] +
                       v[0 * 4 + 3] * v[2 * 4 + 0] * v[3 * 4 + 2] -
                       v[3 * 4 + 0] * v[2 * 4 + 2] * v[0 * 4 + 3] -
                       v[3 * 4 + 2] * v[2 * 4 + 3] * v[0 * 4 + 0] -
                       v[3 * 4 + 3] * v[2 * 4 + 0] * v[0 * 4 + 2]) /
                      d;
        data_[1][2] = -(v[0 * 4 + 0] * v[1 * 4 + 2] * v[3 * 4 + 3] +
                        v[0 * 4 + 2] * v[1 * 4 + 3] * v[3 * 4 + 0] +
                        v[0 * 4 + 3] * v[1 * 4 + 0] * v[3 * 4 + 2] -
                        v[3 * 4 + 0] * v[1 * 4 + 2] * v[0 * 4 + 3] -
                        v[3 * 4 + 2] * v[1 * 4 + 3] * v[0 * 4 + 0] -
                        v[3 * 4 + 3] * v[1 * 4 + 0] * v[0 * 4 + 2]) /
                      d;
        data_[1][3] = (v[0 * 4 + 0] * v[1 * 4 + 2] * v[2 * 4 + 3] +
                       v[0 * 4 + 2] * v[1 * 4 + 3] * v[2 * 4 + 0] +
                       v[0 * 4 + 3] * v[1 * 4 + 0] * v[2 * 4 + 2] -
                       v[2 * 4 + 0] * v[1 * 4 + 2] * v[0 * 4 + 3] -
                       v[2 * 4 + 2] * v[1 * 4 + 3] * v[0 * 4 + 0] -
                       v[2 * 4 + 3] * v[1 * 4 + 0] * v[0 * 4 + 2]) /
                      d;

        data_[2][0] = (v[1 * 4 + 0] * v[2 * 4 + 1] * v[3 * 4 + 3] +
                       v[1 * 4 + 1] * v[2 * 4 + 3] * v[3 * 4 + 0] +
                       v[1 * 4 + 3] * v[2 * 4 + 0] * v[3 * 4 + 1] -
                       v[3 * 4 + 0] * v[2 * 4 + 1] * v[1 * 4 + 3] -
                       v[3 * 4 + 1] * v[2 * 4 + 3] * v[1 * 4 + 0] -
                       v[3 * 4 + 3] * v[2 * 4 + 0] * v[1 * 4 + 1]) /
                      d;
        data_[2][1] = -(v[0 * 4 + 0] * v[2 * 4 + 1] * v[3 * 4 + 3] +
                        v[0 * 4 + 1] * v[2 * 4 + 3] * v[3 * 4 + 0] +
                        v[0 * 4 + 3] * v[2 * 4 + 0] * v[3 * 4 + 1] -
                        v[3 * 4 + 0] * v[2 * 4 + 1] * v[0 * 4 + 3] -
                        v[3 * 4 + 1] * v[2 * 4 + 3] * v[0 * 4 + 0] -
                        v[3 * 4 + 3] * v[2 * 4 + 0] * v[0 * 4 + 1]) /
                      d;
        data_[2][2] = (v[0 * 4 + 0] * v[1 * 4 + 1] * v[3 * 4 + 3] +
                       v[0 * 4 + 1] * v[1 * 4 + 3] * v[3 * 4 + 0] +
                       v[0 * 4 + 3] * v[1 * 4 + 0] * v[3 * 4 + 1] -
                       v[3 * 4 + 0] * v[1 * 4 + 1] * v[0 * 4 + 3] -
                       v[3 * 4 + 1] * v[1 * 4 + 3] * v[0 * 4 + 0] -
                       v[3 * 4 + 3] * v[1 * 4 + 0] * v[0 * 4 + 1]) /
                      d;
        data_[2][3] = -(v[0 * 4 + 0] * v[1 * 4 + 1] * v[2 * 4 + 3] +
                        v[0 * 4 + 1] * v[1 * 4 + 3] * v[2 * 4 + 0] +
                        v[0 * 4 + 3] * v[1 * 4 + 0] * v[2 * 4 + 1] -
                        v[2 * 4 + 0] * v[1 * 4 + 1] * v[0 * 4 + 3] -
                        v[2 * 4 + 1] * v[1 * 4 + 3] * v[0 * 4 + 0] -
                        v[2 * 4 + 3] * v[1 * 4 + 0] * v[0 * 4 + 1]) /
                      d;

        data_[3][0] = -(v[1 * 4 + 0] * v[2 * 4 + 1] * v[3 * 4 + 2] +
                        v[1 * 4 + 1] * v[2 * 4 + 2] * v[3 * 4 + 0] +
                        v[1 * 4 + 2] * v[2 * 4 + 0] * v[3 * 4 + 1] -
                        v[3 * 4 + 0] * v[2 * 4 + 1] * v[1 * 4 + 2] -
                        v[3 * 4 + 1] * v[2 * 4 + 2] * v[1 * 4 + 0] -
                        v[3 * 4 + 2] * v[2 * 4 + 0] * v[1 * 4 + 1]) /
                      d;
        data_[3][1] = (v[0 * 4 + 0] * v[2 * 4 + 1] * v[3 * 4 + 2] +
                       v[0 * 4 + 1] * v[2 * 4 + 2] * v[3 * 4 + 0] +
                       v[0 * 4 + 2] * v[2 * 4 + 0] * v[3 * 4 + 1] -
                       v[3 * 4 + 0] * v[2 * 4 + 1] * v[0 * 4 + 2] -
                       v[3 * 4 + 1] * v[2 * 4 + 2] * v[0 * 4 + 0] -
                       v[3 * 4 + 2] * v[2 * 4 + 0] * v[0 * 4 + 1]) /
                      d;
        data_[3][2] = -(v[0 * 4 + 0] * v[1 * 4 + 1] * v[3 * 4 + 2] +
                        v[0 * 4 + 1] * v[1 * 4 + 2] * v[3 * 4 + 0] +
                        v[0 * 4 + 2] * v[1 * 4 + 0] * v[3 * 4 + 1] -
                        v[3 * 4 + 0] * v[1 * 4 + 1] * v[0 * 4 + 2] -
                        v[3 * 4 + 1] * v[1 * 4 + 2] * v[0 * 4 + 0] -
                        v[3 * 4 + 2] * v[1 * 4 + 0] * v[0 * 4 + 1]) /
                      d;
        data_[3][3] = (v[0 * 4 + 0] * v[1 * 4 + 1] * v[2 * 4 + 2] +
                       v[0 * 4 + 1] * v[1 * 4 + 2] * v[2 * 4 + 0] +
                       v[0 * 4 + 2] * v[1 * 4 + 0] * v[2 * 4 + 1] -
                       v[2 * 4 + 0] * v[1 * 4 + 1] * v[0 * 4 + 2] -
                       v[2 * 4 + 1] * v[1 * 4 + 2] * v[0 * 4 + 0] -
                       v[2 * 4 + 2] * v[1 * 4 + 0] * v[0 * 4 + 1]) /
                      d;
    }
}

template <class T> inline Matrix4<T> Matrix4<T>::inverted() const
{
    Matrix4<T> m(*this);
    m.invert();
    return m;
}

template <class T> inline void Matrix4<T>::translate(T x, T y, T z)
{
    Matrix4<T> m1(*this);
    Matrix4<T> m2;
    m2.data_[3][0] = x;
    m2.data_[3][1] = y;
    m2.data_[3][2] = z;
    *this = m1 * m2;
}

template <class T>
inline void Matrix4<T>::perspective(T fovy, T aspect, T near, T far)
{
    double fov = static_cast<double>(fovy) * 3.1415927 / 180.0;
    T f = static_cast<T>(1.0 / std::tan(fov * 0.5));

    data_[0][0] = f / aspect;
    data_[1][1] = f;
    // data_[2][2] = (far + near) / (near - far);
    // data_[3][2] = (2 * far * near) / (near - far);
    data_[2][3] = -1;
    data_[3][3] = 0;
}

template <class T>
inline void Matrix4<T>::ortho(T left, T right, T bottom, T top, T near, T far)
{
    data_[0][0] = 2 / (right - left);
    data_[1][1] = 2 / (top - bottom);
    // data_[2][2] = -2 / (far - near);
    data_[3][0] = -((right + left) / (right - left));
    data_[3][1] = -((top + bottom) / (top - bottom));
    // data_[3][2] = -((far + near) / (far - near));
}

template <class T>
void Matrix4<T>::lookAt(const Vector3<T> &eye,
                        const Vector3<T> &center,
                        const Vector3<T> &up)
{
    Vector3<T> v = center - eye;
    v.normalize();

    Vector3<T> r = v.crossProduct(up);
    r.normalize();

    Vector3<T> u = r.crossProduct(v);

    T x = -eye(0);
    T y = -eye(1);
    T z = -eye(2);

    data_[0][0] = r(0);
    data_[1][0] = r(1);
    data_[2][0] = r(2);
    data_[3][0] = data_[0][0] * x + data_[1][0] * y + data_[2][0] * z;
    data_[0][1] = u(0);
    data_[1][1] = u(1);
    data_[2][1] = u(2);
    data_[3][1] = data_[0][1] * x + data_[1][1] * y + data_[2][1] * z;
    data_[0][2] = -v(0);
    data_[1][2] = -v(1);
    data_[2][2] = -v(2);
    data_[3][2] = data_[0][2] * x + data_[1][2] * y + data_[2][2] * z;
    data_[0][3] = 0;
    data_[1][3] = 0;
    data_[2][3] = 0;
    data_[3][3] = 1 + data_[0][3] * x + data_[1][3] * y + data_[2][3] * z;
}

template <class T> Vector3<T> Matrix4<T>::map(const Vector3<T> &v)
{
    T x = data_[0][0] * v(0) + data_[1][0] * v(1) + data_[2][0] * v(2) +
          data_[3][0];

    T y = data_[0][1] * v(0) + data_[1][1] * v(1) + data_[2][1] * v(2) +
          data_[3][1];

    T z = data_[0][2] * v(0) + data_[1][2] * v(1) + data_[2][2] * v(2) +
          data_[3][2];

    T w = data_[0][3] * v(0) + data_[1][3] * v(1) + data_[2][3] * v(2) +
          data_[3][3];

    constexpr T e = std::numeric_limits<T>::epsilon();
    if (w > e || w < -e)
    {
        return Vector3<T>(x / w, y / w, z / w);
    }
    else
    {
        return Vector3<T>(x, y, z);
    }
}

template <class T> Vector3<T> Matrix4<T>::mapVector(const Vector3<T> &v)
{
    return Vector3<T>(
        data_[0][0] * v(0) + data_[1][0] * v(1) + data_[2][0] * v(2),
        data_[0][1] * v(0) + data_[1][1] * v(1) + data_[2][1] * v(2),
        data_[0][2] * v(0) + data_[1][2] * v(1) + data_[2][2] * v(2));
}

template <class T> void Matrix4<T>::multiply(T &x, T &y, T &z, T &w)
{
    T x1 = x;
    T y1 = y;
    T z1 = z;
    T w1 = w;

    x = x1 * data_[0][0] + y1 * data_[1][0] + z1 * data_[2][0] +
        w1 * data_[3][0];
    y = x1 * data_[0][1] + y1 * data_[1][1] + z1 * data_[2][1] +
        w1 * data_[3][1];
    z = x1 * data_[0][2] + y1 * data_[1][2] + z1 * data_[2][2] +
        w1 * data_[3][2];
    w = x1 * data_[0][3] + y1 * data_[1][3] + z1 * data_[2][3] +
        w1 * data_[3][3];
}

template <class T> Vector3<T> Matrix4<T>::project(const Vector3<T> &v)
{
    T x = v.x();
    T y = v.y();
    T z = v.z();
    T w = 1;

    multiply(x, y, z, w);

    constexpr T e = std::numeric_limits<T>::epsilon();
    if (w > -e && w < e)
    {
        w = 1;
    }

    constexpr T c = static_cast<T>(0.5);
    x = x / w * c + c;
    y = y / w * c + c;
    z = z / w * c + c;

    return Vector3<T>(x, y, z);
}

template <class T> Vector3<T> Matrix4<T>::unproject(const Vector3<T> &v)
{
    T x = v.x() * 2 - 1;
    T y = v.y() * 2 - 1;
    T z = v.z() * 2 - 1;
    T w = 1;

    multiply(x, y, z, w);

    constexpr T e = std::numeric_limits<T>::epsilon();
    if (w > e || w < -e)
    {
        x /= w;
        y /= w;
        z /= w;
    }

    return Vector3<T>(x, y, z);
}

#endif /* MATRIX4_HPP */