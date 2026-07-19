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
#include <Util.hpp>

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

Color::Color(Ui::GlobalColor gc)
{
}

void Color::setRed(int r)
{
    r_ = r;
    clamp(r_, 0, 255);
    spec_ = Color::Rgb;
}

void Color::setGreen(int g)
{
    g_ = g;
    clamp(g_, 0, 255);
    spec_ = Color::Rgb;
}

void Color::setBlue(int b)
{
    b_ = b;
    clamp(b_, 0, 255);
    spec_ = Color::Rgb;
}

void Color::setAlpha(int a)
{
    a_ = a;
    clamp(a_, 0, 255);
}

void Color::setRedF(float r)
{
    setRed(static_cast<int>(r * 255.0F));
}

void Color::setGreenF(float g)
{
    setGreen(static_cast<int>(g * 255.0F));
}

void Color::setBlueF(float b)
{
    setBlue(static_cast<int>(b * 255.0F));
}

void Color::setAlphaF(float a)
{
    setAlpha(static_cast<int>(a * 255.0F));
}

void Color::setRgb(int r, int g, int b, int a)
{
    setRed(r);
    setGreen(g);
    setBlue(b);
    setAlpha(a);
}

void Color::setRgbF(float r, float g, float b, float a)
{
    setRedF(r);
    setGreenF(g);
    setBlueF(b);
    setAlphaF(a);
}
