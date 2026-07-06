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

/** @file Painter.cpp */

// Include 3D Forest.
#include <Painter.hpp>
#include <Widget.hpp>

// Include local.
#define LOG_MODULE_NAME "Painter"
#include <Log.hpp>

Painter::Painter()
{
}

Painter::Painter(Widget *widget)
{
}

Painter::Painter(int w, int h)
   : w_(w), h_(h)
{
}

void Painter::setWidth(int w)
{
    w_ = w;
}

void Painter::setHeight(int h)
{
    h_ = h;
}

void Painter::setPen(const Color &color)
{
}

void Painter::drawLine(int x1, int y1, int x2, int y2)
{
}

void Painter::drawRect(int x1, int y1, int w, int h)
{
}

void Painter::fillRect(int x, int y, int w, int h, const Color &color)
{
}
