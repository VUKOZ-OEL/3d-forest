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

/** @file Slider.cpp */

// Include std.

// Include 3D Forest.
#include <Slider.hpp>
#include <Application.hpp>

// Include local.
#define LOG_MODULE_NAME "Slider"
#include <Log.hpp>

Slider::Slider()
{
}

Slider::~Slider()
{
}

void Slider::setMinimum(int v)
{
}

void Slider::setMaximum(int v)
{
}

void Slider::setSingleStep(int v)
{
}

void Slider::setTickInterval(int v)
{
}

void Slider::setTickPosition(int v)
{
}

void Slider::setOrientation(int v)
{
}

void Slider::setValue(int value, bool notify)
{
    if (value_ == value)
    {
        return;
    }

    value_ = value;

    if (notify && !signalsBlocked())
    {
        valueChanged(value_);
    }
}
