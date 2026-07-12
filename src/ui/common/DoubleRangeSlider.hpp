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

/** @file DoubleRangeSlider.hpp */

#ifndef DOUBLE_RANGE_SLIDER_HPP
#define DOUBLE_RANGE_SLIDER_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** DoubleRangeSlider. */
class EXPORT_UI_COMMON DoubleRangeSlider : public Widget
{
public:
    DoubleRangeSlider();
    virtual ~DoubleRangeSlider();

    double singleStep() const { return singleStep_; }
    void setSingleStep(double val);

    void setOrientation(int v);

    double minimum() const { return minimum_; }
    void setMinimum(double min);

    double maximum() const { return maximum_; }
    void setMaximum(double max);

    void setRange(double min, double max);

    double minimumValue() const { return minimumValue_; }
    void setMinimumValue(double value, bool notify = false);

    double maximumValue() const { return maximumValue_; }
    void setMaximumValue(double value, bool notify = false);

    void setValues(double minVal, double maxVal, bool notify = false);

    Signal<double> minimumValueChanged;
    Signal<double> maximumValueChanged;
    Signal<> sliderReleased;

private:
    double singleStep_{0};
    double minimum_{0};
    double maximum_{0};
    double minimumValue_{0};
    double maximumValue_{0};
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_RANGE_SLIDER_HPP */
