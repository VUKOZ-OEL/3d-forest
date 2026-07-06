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

/** @file Point.hpp */

#ifndef POINT_HPP
#define POINT_HPP

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Point. */
class EXPORT_UI_COMMON Point
{
public:
    Point();
    Point(int x, int y);

    int x() const { return x_; }
    int y() const { return y_; }

private:
    int x_{0};
    int y_{0};
};

#include <WarningsEnable.hpp>

#endif /* POINT_HPP */
