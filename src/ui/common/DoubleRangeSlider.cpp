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

/** @file DoubleRangeSlider.cpp */

// Include std.
#include <algorithm>
#include <cmath>
#include <utility>

// Include 3D Forest.
#include <Application.hpp>
#include <DoubleRangeSlider.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "DoubleRangeSlider"
#include <Log.hpp>

DoubleRangeSlider::DoubleRangeSlider()
{
}

DoubleRangeSlider::~DoubleRangeSlider()
{
}

void DoubleRangeSlider::setSingleStep(double value)
{
    if (!std::isfinite(value) || value <= 0.0)
    {
        return;
    }

    if (singleStep_ == value)
    {
        return;
    }

    singleStep_ = value;
    settingsChanged();
}

void DoubleRangeSlider::setOrientation(int orientation)
{
    if (orientation != Horizontal && orientation != Vertical)
    {
        return;
    }

    if (orientation_ == orientation)
    {
        return;
    }

    orientation_ = orientation;
    settingsChanged();
}

void DoubleRangeSlider::setMinimum(double minimum)
{
    if (!std::isfinite(minimum))
    {
        return;
    }

    setRange(minimum, std::max(minimum, maximum_));
}

void DoubleRangeSlider::setMaximum(double maximum)
{
    if (!std::isfinite(maximum))
    {
        return;
    }

    setRange(std::min(minimum_, maximum), maximum);
}

void DoubleRangeSlider::setRange(double minimum, double maximum)
{
    if (!std::isfinite(minimum) || !std::isfinite(maximum))
    {
        return;
    }

    maximum = std::max(minimum, maximum);

    if (minimum_ == minimum && maximum_ == maximum)
    {
        return;
    }

    minimum_ = minimum;
    maximum_ = maximum;

    const double previousMinimumValue = minimumValue_;
    const double previousMaximumValue = maximumValue_;

    minimumValue_ = std::clamp(minimumValue_, minimum_, maximum_);
    maximumValue_ = std::clamp(maximumValue_, minimum_, maximum_);

    settingsChanged();

    if (minimumValue_ != previousMinimumValue ||
        maximumValue_ != previousMaximumValue)
    {
        valuesUpdated(minimumValue_, maximumValue_);
    }
}

void DoubleRangeSlider::setMinimumValue(double value, bool notify)
{
    if (!std::isfinite(value))
    {
        return;
    }

    // Move the upper value too if necessary to preserve ordering.
    setValues(value, std::max(value, maximumValue_), notify);
}

void DoubleRangeSlider::setMaximumValue(double value, bool notify)
{
    if (!std::isfinite(value))
    {
        return;
    }

    // Move the lower value too if necessary to preserve ordering.
    setValues(std::min(minimumValue_, value), value, notify);
}

void DoubleRangeSlider::setValues(double minimumValue,
                                  double maximumValue,
                                  bool notify)
{
    if (!std::isfinite(minimumValue) || !std::isfinite(maximumValue))
    {
        return;
    }

    if (minimumValue > maximumValue)
    {
        std::swap(minimumValue, maximumValue);
    }

    minimumValue = std::clamp(minimumValue, minimum_, maximum_);
    maximumValue = std::clamp(maximumValue, minimum_, maximum_);

    const bool minimumChanged = minimumValue_ != minimumValue;
    const bool maximumChanged = maximumValue_ != maximumValue;

    if (!minimumChanged && !maximumChanged)
    {
        return;
    }

    // Store both values before notifying subscribers.
    minimumValue_ = minimumValue;
    maximumValue_ = maximumValue;

    valuesUpdated(minimumValue_, maximumValue_);

    if (notify && !signalsBlocked())
    {
        if (minimumChanged)
        {
            minimumValueChanged(minimumValue_);
        }

        if (maximumChanged)
        {
            maximumValueChanged(maximumValue_);
        }
    }
}