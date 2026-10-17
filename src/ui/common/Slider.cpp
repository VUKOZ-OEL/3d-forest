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
#include <algorithm>
#include <stdexcept>

// Include 3D Forest.
#include <Application.hpp>
#include <Slider.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "Slider"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

Slider::Slider()
{
}

Slider::~Slider()
{
}

void Slider::setSingleStep(int value)
{
    value = std::max(1, value);

    if (singleStep_ == value)
    {
        return;
    }

    singleStep_ = value;
    settingsChanged();
}

void Slider::setTickInterval(int value)
{
    value = std::max(0, value);

    if (tickInterval_ == value)
    {
        return;
    }

    tickInterval_ = value;
    settingsChanged();
}

void Slider::setTickPosition(int value)
{
    if (value < NoTicks || value > TicksBothSides)
    {
        throw std::invalid_argument("Invalid tick position");
    }

    if (tickPosition_ == value)
    {
        return;
    }

    tickPosition_ = static_cast<TickPosition>(value);
    settingsChanged();
}

void Slider::setOrientation(int orientation)
{
    if (orientation != Ui::Horizontal && orientation != Ui::Vertical)
    {
        throw std::invalid_argument("Invalid slider orientation");
    }
    if (orientation_ == orientation)
    {
        return;
    }
    orientation_ = orientation;
    settingsChanged();
}

void Slider::setMinimum(int value)
{
    setRange(value, (std::max)(value, maximum_));
}

void Slider::setMaximum(int value)
{
    setRange((std::min)(minimum_, value), value);
}

void Slider::setRange(int minimum, int maximum)
{
    maximum = (std::max)(minimum, maximum);
    if (minimum_ == minimum && maximum_ == maximum)
        return;
    minimum_ = minimum;
    maximum_ = maximum;
    const int oldValue = value_;
    value_ = (std::max)(minimum_, (std::min)(value_, maximum_));
    const auto guard = lifetime();
    settingsChanged();
    if (!guard.expired() && value_ != oldValue)
        valueUpdated(value_);
}

void Slider::setValue(int value, bool notify)
{
    LOG_DEBUG(<< "setValue <" << value << ">.");

    value = (std::max)(minimum_, (std::min)(value, maximum_));

    if (value_ == value)
    {
        return;
    }

    value_ = value;
    const auto guard = lifetime();
    valueUpdated(value_);

    if (!guard.expired() && notify && !signalsBlocked())
    {
        valueChanged(value_);
    }
}
