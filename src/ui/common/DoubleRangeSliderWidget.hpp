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

/** @file DoubleRangeSliderWidget.hpp */

#ifndef DOUBLE_RANGE_SLIDER_WIDGET_HPP
#define DOUBLE_RANGE_SLIDER_WIDGET_HPP

// Include 3D Forest.
#include <Widget.hpp>
class DoubleRangeSlider;
class DoubleSpinBox;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** DoubleRangeSliderWidget. */
class EXPORT_UI_COMMON DoubleRangeSliderWidget : public Widget
{
public:
    DoubleRangeSliderWidget();
    virtual ~DoubleRangeSliderWidget();

    static void create(DoubleRangeSliderWidget *&outputWidget,
                       std::function<void(double)> minValueChangedCallback = {},
                       std::function<void(double)> maxValueChangedCallback = {},
                       std::function<void()> finalValueCallback = {},
                       const std::string &text,
                       const std::string &toolTip,
                       const std::string &unitsList,
                       double step,
                       double min,
                       double max,
                       double minValue,
                       double maxValue);

    double minimum() const;
    void setMinimum(double v);

    double maximum() const;
    void setMaximum(double v);

    void setRange(double min, double max);

    double minimumValue() const;
    void setMinimumValue(double value, bool notify = false);

    double maximumValue() const;
    void setMaximumValue(double value, bool notify = false);

    Signal<double> minimumValueChanged;
    Signal<double> maximumValueChanged;
    Signal<> finalValue;

    void slotMinimumValueChangedSlider(double v);
    void slotMaximumValueChangedSlider(double v);
    void slotMinimumValueChangedSpinBox(double v);
    void slotMaximumValueChangedSpinBox(double v);
    void slotFinalValueSlider();
    void slotFinalValueSpinBox();

private:
    DoubleRangeSlider *slider_;
    DoubleSpinBox *minSpinBox_;
    DoubleSpinBox *maxSpinBox_;
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_RANGE_SLIDER_WIDGET_HPP */
