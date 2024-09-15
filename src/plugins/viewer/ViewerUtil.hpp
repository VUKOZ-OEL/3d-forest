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

/** @file ViewerUtil.hpp */

#ifndef VIEWER_UTIL_HPP
#define VIEWER_UTIL_HPP

// Include 3D Forest.
#include <Json.hpp>

// Include Qt.
#include <QVector3D>

// Include local.
#include <WarningsDisable.hpp>

inline void toJson(Json &out, const QVector3D &in)
{
    toJson(out[0], in[0]);
    toJson(out[1], in[1]);
    toJson(out[2], in[2]);
}

inline std::string toString(const QVector3D &in)
{
    return "[" + toString(in[0]) + "," + toString(in[1]) + "," +
           toString(in[2]) + "]";
}

inline std::ostream &operator<<(std::ostream &out, const QVector3D &in)
{
    return out << std::fixed << std::setprecision(15) << "[" << in[0] << ","
               << in[1] << "," << in[2] << "]" << std::defaultfloat;
}

#include <WarningsEnable.hpp>

#endif /* VIEWER_UTIL_HPP */
