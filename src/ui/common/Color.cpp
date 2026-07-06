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

/** @file Color.cpp */

// Include 3D Forest.
#include <Color.hpp>

// Include local.
#define LOG_MODULE_NAME "Color"
#include <Log.hpp>

Color::Color()
{
}

Color::Color(int r, int g, int b, int a)
{
    setRgb(r, g, b, a);
}

Color::~Color()
{
}

void Color::setRgb(int r, int g, int b, int a)
{
    r_ = r;
    g_ = g;
    b_ = b;
    a_ = a;
}

void Color::setRgbF(float r, float g, float b, float a)
{
    setRgb(static_cast<int>(r * 255.0F),
           static_cast<int>(g * 255.0F),
           static_cast<int>(b * 255.0F),
           static_cast<int>(a * 255.0F));
}
