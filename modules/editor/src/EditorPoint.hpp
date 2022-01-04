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

/** @file EditorPoint.hpp */

#ifndef EDITOR_POINT_HPP
#define EDITOR_POINT_HPP

#include <Json.hpp>

/** Editor Point. */
class EditorPoint
{
public:
    double x;
    double y;
    double z;
    float intensity;
    uint8_t returnNumber;
    uint8_t numberOfReturns;
    uint8_t classification;
    uint8_t userData;
    double gpsTime;
    float red;
    float green;
    float blue;
    float userRed;
    float userGreen;
    float userBlue;
    uint32_t layer;

    EditorPoint();
    ~EditorPoint();

    Json &write(Json &out) const;
};

std::ostream &operator<<(std::ostream &os, const EditorPoint &obj);

#endif /* EDITOR_POINT_HPP */
