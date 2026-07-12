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

// Include 3D Forest.
#include <DoubleRangeSlider.hpp>
#include <Application.hpp>
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

void DoubleRangeSlider::setSingleStep(double val)
{
    singleStep_ = val;
}

void DoubleRangeSlider::setOrientation(int v)
{
}

void DoubleRangeSlider::setMinimum(double min)
{
    minimum_ = min;
    minimumValue_ = min;
}

void DoubleRangeSlider::setMaximum(double max)
{
    maximum_ = max;
    maximumValue_ = max;
}

void DoubleRangeSlider::setRange(double min, double max)
{
    setMinimum(min);
    setMaximum(max);
}

void DoubleRangeSlider::setMinimumValue(double value, bool notify)
{
    double newMinimum = value;
    clamp(newMinimum, minimum_, maximum_);
    bool minChanged = !equal(newMinimum, minimumValue_);

    minimumValue_ = newMinimum;

    if (minChanged && notify && !signalsBlocked())
    {
        minimumValueChanged(minimumValue_);
    }
}

void DoubleRangeSlider::setMaximumValue(double value, bool notify)
{
    double newMaximum = value;
    clamp(newMaximum, minimum_, maximum_);
    bool maxChanged = !equal(newMaximum, minimumValue_);

    maximumValue_ = newMaximum;

    if (maxChanged && notify && !signalsBlocked())
    {
        maximumValueChanged(maximumValue_);
    }
}

void DoubleRangeSlider::setValues(double minVal, double maxVal, bool notify)
{
    double newMinimum = minVal;
    clamp(newMinimum, minimum_, maximum_);
    bool minChanged = !equal(newMinimum, minimumValue_);
    minimumValue_ = newMinimum;

    double newMaximum = maxVal;
    clamp(newMaximum, minimum_, maximum_);
    bool maxChanged = !equal(newMaximum, minimumValue_);
    maximumValue_ = newMaximum;

    if (notify && !signalsBlocked())
    {
        if (minChanged)
        {
            minimumValueChanged(minimumValue_);
        }

        if (maxChanged)
        {
            maximumValueChanged(maximumValue_);
        }
    }
}

#if 0
    // Include 3rd party.
    #include <ctkDoubleRangeSlider.h>

    ctkDoubleRangeSlider *slider_;

    slider_->setMinimum(min);
    slider_->setMaximum(max);
    slider_->setMinimumValue(value);
    slider_->setMaximumValue(value);

    slider_->disconnectSlider();
    slider_->connectSlider();

    slider_->blockSignals(true);
    slider_->setMinimumValue(v);
    slider_->blockSignals(false);

    // Value Slider.
    ctkDoubleRangeSlider *slider = new ctkDoubleRangeSlider;
    slider->setRange(min, max);
    slider->setValues(minValue, maxValue);
    slider->setSingleStep(step);
    slider->setOrientation(Qt::Horizontal);

    connect(slider,
            SIGNAL(minimumPositionChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMinimumValue(double)));

    connect(slider,
            SIGNAL(maximumPositionChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMaximumValue(double)));

    connect(slider,
            SIGNAL(sliderReleased()),
            outputWidget,
            SLOT(slotFinalValue()));

#endif
