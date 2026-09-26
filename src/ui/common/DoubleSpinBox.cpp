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

/** @file DoubleSpinBox.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <DoubleSpinBox.hpp>

// Include local.
#define LOG_MODULE_NAME "DoubleSpinBox"
#include <Log.hpp>

DoubleSpinBox::DoubleSpinBox()
{
}

DoubleSpinBox::~DoubleSpinBox()
{
}

void DoubleSpinBox::setSingleStep(double value)
{
    if (value < 0 || singleStep_ == value)
    {
        return;
    }

    singleStep_ = value;
    settingsChanged();
}

void DoubleSpinBox::setMinimum(double minimum)
{
    setRange(minimum, std::max(minimum, maximum_));
}

void DoubleSpinBox::setMaximum(double maximum)
{
    setRange(std::min(minimum_, maximum), maximum);
}

void DoubleSpinBox::setRange(double minimum, double maximum)
{
    // Ensure a valid range.
    maximum = std::max(minimum, maximum);

    if (minimum_ == minimum && maximum_ == maximum)
    {
        return;
    }

    minimum_ = minimum;
    maximum_ = maximum;

    const double previousValue = value_;
    clamp(value_, minimum_, maximum_);

    settingsChanged();

    if (value_ != previousValue)
    {
        valueUpdated(value_);
    }
}

void DoubleSpinBox::setValue(double value, bool notify)
{
    clamp(value, minimum_, maximum_);

    if (value_ == value)
    {
        return;
    }

    value_ = value;
    valueUpdated(value_);

    if (notify && !signalsBlocked())
    {
        valueChanged(value_);
    }
}