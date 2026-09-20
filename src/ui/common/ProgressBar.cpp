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

/** @file ProgressBar.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <ProgressBar.hpp>

// Include local.
#define LOG_MODULE_NAME "ProgressBar"
#include <Log.hpp>

ProgressBar::ProgressBar()
{
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::setRange(int minimum, int maximum)
{
    if (minimum < 0 || maximum < minimum)
    {
        throw std::invalid_argument("Invalid progress range");
    }

    if (minimum_ == minimum && maximum_ == maximum)
    {
        return;
    }

    minimum_ = minimum;
    maximum_ = maximum;

    const bool resetValue = value_ < minimum_ || value_ > maximum_;

    if (resetValue)
    {
        value_ = minimum_ - 1;
    }

    rangeChanged(minimum_, maximum_);

    if (resetValue)
    {
        resetRequested();
    }
}

void ProgressBar::setValue(int value)
{
    if (value < minimum_ || value > maximum_)
    {
        return;
    }

    if (value_ == value)
    {
        return;
    }

    value_ = value;
    valueChanged(value_);
}

void ProgressBar::reset()
{
    value_ = minimum_ - 1;
    resetRequested();
}

void ProgressBar::setTextVisible(bool visible)
{
    if (textVisible_ == visible)
    {
        return;
    }

    textVisible_ = visible;
    textVisibleChanged(textVisible_);
}