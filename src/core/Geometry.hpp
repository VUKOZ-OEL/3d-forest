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

// Include std.
#include <cmath>

// Include 3D Forest.
#include <Vector3.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

template <class T> inline T distance(T ax, T ay, T az, T bx, T by, T bz)
{
    T dx = bx - ax;
    T dy = by - ay;
    T dz = bz - az;

    return static_cast<T>(std::sqrt((dx * dx) + (dy * dy) + (dz * dz)));
}

template <class T>
inline T pointLineDistance(T x, T y, T z, T ax, T ay, T az, T bx, T by, T bz)
{
    T abx = bx - ax;
    T aby = by - ay;
    T abz = bz - az;

    T apx = x - ax;
    T apy = y - ay;
    T apz = z - az;

    T abap = abx * apx + aby * apy + abz * apz;
    T abab = abx * abx + aby * aby + abz * abz;

    if (zero(abab))
    {
        return static_cast<T>(std::sqrt(apx * apx + apy * apy + apz * apz));
    }

    T t = abap / abab;
    if (t < 0)
    {
        t = 0;
    }
    else if (t > 1)
    {
        t = 1;
    }

    T dx = x - (ax + abx * t);
    T dy = y - (ay + aby * t);
    T dz = z - (az + abz * t);

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

template <class T>
inline bool intersectSegmentAABB(T ax,
                                 T ay,
                                 T az,
                                 T bx,
                                 T by,
                                 T bz,
                                 T minx,
                                 T miny,
                                 T minz,
                                 T maxx,
                                 T maxy,
                                 T maxz,
                                 T &isx,
                                 T &isy,
                                 T &isz)
{
    T tmin = 0;
    T tmax = 1;

    T dx = bx - ax;
    if (std::abs(dx) < 1e-8)
    {
        if (ax < minx || ax > maxx)
        {
            return false;
        }
    }
    else
    {
        T invD = static_cast<T>(1.0 / static_cast<double>(dx));
        T t1 = (minx - ax) * invD;
        T t2 = (maxx - ax) * invD;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);

        if (tmin > tmax)
        {
            return false;
        }
    }

    T dy = by - ay;
    if (std::abs(dy) < 1e-8)
    {
        if (ay < miny || ay > maxy)
        {
            return false;
        }
    }
    else
    {
        T invD = static_cast<T>(1.0 / static_cast<double>(dy));
        T t1 = (miny - ay) * invD;
        T t2 = (maxy - ay) * invD;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);

        if (tmin > tmax)
        {
            return false;
        }
    }

    T dz = bz - az;
    if (std::abs(dz) < 1e-8)
    {
        if (az < minz || az > maxz)
        {
            return false;
        }
    }
    else
    {
        T invD = static_cast<T>(1.0 / static_cast<double>(dz));
        T t1 = (minz - az) * invD;
        T t2 = (maxz - az) * invD;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);

        if (tmin > tmax)
        {
            return false;
        }
    }

    isx = ax + tmin * (bx - ax);
    isy = ay + tmin * (by - ay);
    isz = az + tmin * (bz - az);

    return true;
}

#include <WarningsEnable.hpp>

#endif /* GEOMETRY_HPP */
