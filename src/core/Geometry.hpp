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

/** @file Geometry.hpp */

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>

#include <Vector3.hpp>

#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

template <class T>
inline T pointLineDistance(T x, T y, T z, T ax, T ay, T az, T nx, T ny, T nz)
{
    T bx = ax + nx;
    T by = ay + ny;
    T bz = az + nz;

    T dx = (y - ay) * (z - bz) - (z - az) * (y - by);
    T dy = (z - az) * (x - bx) - (x - ax) * (z - bz);
    T dz = (x - ax) * (y - by) - (y - ay) * (x - bx);

    return static_cast<T>(std::sqrt((dx * dx) + (dy * dy) + (dz * dz)));
}

template <class T>
inline T pointPlaneDistance(const Vector3<T> &x,
                            const Vector3<T> &p,
                            const Vector3<T> &n)
{
    return Vector3<T>::dotProduct(x, n) - Vector3<T>::dotProduct(p, n);
}

template <class T>
inline T pointPlaneDistance(T x, T y, T z, T px, T py, T pz, T nx, T ny, T nz)
{
    return ((x * nx) + (y * ny) + (z * nz)) -
           ((px * nx) + (py * ny) + (pz * nz));
}

#include <WarningsEnable.hpp>

#endif /* GEOMETRY_HPP */
