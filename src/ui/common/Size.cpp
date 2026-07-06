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

/** @file Size.cpp */

// Include 3D Forest.
#include <Size.hpp>

// Include local.
#define LOG_MODULE_NAME "Size"
#include <Log.hpp>

Size::Size()
{
}

Size::Size(int w, int h)
   : w_(w), h_(h)
{
}

void Size::setWidth(int w)
{
    w_ = w;
}

void Size::setHeight(int h)
{
    h_ = h;
}
