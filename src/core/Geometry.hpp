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
bool intersectSegmentSphere(T x1,
                            T y1,
                            T z1, // segment start
                            T x2,
                            T y2,
                            T z2, // segment end
                            T sx,
                            T sy,
                            T sz,     // sphere center
                            T radius, // sphere radius (>=0)
                            T *cp_x = nullptr,
                            T *cp_y = nullptr,
                            T *cp_z = nullptr // optional out collision point
)
{
    static_assert(std::is_floating_point<T>::value, "T must be floating point");
    if (radius < static_cast<T>(0))
    {
        return false;
    }

    const T EPS = std::numeric_limits<T>::epsilon() * static_cast<T>(100);

    // ab = x2 - x1
    T abx = x2 - x1;
    T aby = y2 - y1;
    T abz = z2 - z1;

    // as = s - x1
    T asx = sx - x1;
    T asy = sy - y1;
    T asz = sz - z1;

    // center of segment c = x1 + ab*0.5
    T cx = x1 + abx * static_cast<T>(0.5);
    T cy = y1 + aby * static_cast<T>(0.5);
    T cz = z1 + abz * static_cast<T>(0.5);

    // sc = c - s
    T scx = cx - sx;
    T scy = cy - sy;
    T scz = cz - sz;

    T ab_sqr = abx * abx + aby * aby + abz * abz;
    T ab_norm = std::sqrt(ab_sqr);
    T sc_norm = std::sqrt(scx * scx + scy * scy + scz * scz);

    T half_len = ab_norm * static_cast<T>(0.5);

    // quick reject using segment midpoint
    if (sc_norm > half_len + radius + EPS)
    {
        return false;
    }

    // projection parameter ti = ab.dot(as) / ab.normSqr()
    if (ab_sqr <= EPS)
    { // degenerate segment -> point test
        T dx = x1 - sx;
        T dy = y1 - sy;
        T dz = z1 - sz;
        T dist2 = dx * dx + dy * dy + dz * dz;
        if (dist2 <= radius * radius + EPS)
        {
            if (cp_x)
                *cp_x = x1;
            if (cp_y)
                *cp_y = y1;
            if (cp_z)
                *cp_z = z1;
            return true;
        }
        return false;
    }

    T bdot = abx * asx + aby * asy + abz * asz;
    T ti = bdot / ab_sqr;

    // point on infinite line
    T px = x1 + abx * ti;
    T py = y1 + aby * ti;
    T pz = z1 + abz * ti;

    // distance from this point to sphere center
    T dx = px - sx;
    T dy = py - sy;
    T dz = pz - sz;
    T dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist <= radius + EPS)
    {
        if (cp_x)
            *cp_x = px;
        if (cp_y)
            *cp_y = py;
        if (cp_z)
            *cp_z = pz;
        return true;
    }

    return false;
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
