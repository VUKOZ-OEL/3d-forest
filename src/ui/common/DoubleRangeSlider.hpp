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
    enum Orientation
    {
        Horizontal = 1,
        Vertical = 2
    };

    DoubleRangeSlider();
    virtual ~DoubleRangeSlider();

    double singleStep() const { return singleStep_; }
    void setSingleStep(double value);

    int orientation() const { return orientation_; }
    void setOrientation(int orientation);

    double minimum() const { return minimum_; }
    void setMinimum(double minimum);

    double maximum() const { return maximum_; }
    void setMaximum(double maximum);

    void setRange(double minimum, double maximum);

    double minimumValue() const { return minimumValue_; }
    void setMinimumValue(double value, bool notify = false);

    double maximumValue() const { return maximumValue_; }
    void setMaximumValue(double value, bool notify = false);

    void setValues(double minimumValue,
                   double maximumValue,
                   bool notify = false);

    Signal<> settingsChanged;
    Signal<double, double> valuesUpdated;

    Signal<double> minimumValueChanged;
    Signal<double> maximumValueChanged;
    Signal<> sliderReleased;

private:
    double singleStep_{1.0};
    int orientation_{Horizontal};

    double minimum_{0.0};
    double maximum_{100.0};

    double minimumValue_{0.0};
    double maximumValue_{100.0};
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_RANGE_SLIDER_HPP */
