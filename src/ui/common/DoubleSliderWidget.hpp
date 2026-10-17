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

/** @file DoubleSliderWidget.hpp */

#ifndef DOUBLE_SLIDER_WIDGET_HPP
#define DOUBLE_SLIDER_WIDGET_HPP

// Include 3D Forest.
#include <Widget.hpp>
class Slider;
class DoubleSpinBox;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** DoubleSliderWidget. */
class EXPORT_UI_COMMON DoubleSliderWidget : public Widget
{
public:
    /** Slider layout. */
    enum Layout
    {
        LAYOUT_SLIDER_BESIDE_LABEL,
        LAYOUT_SLIDER_UNDER_LABEL
    };

    DoubleSliderWidget();
    virtual ~DoubleSliderWidget();

    static void create(DoubleSliderWidget *&outputWidget,
                       std::function<void(double)> valueChangedCallback,
                       std::function<void()> finalValueCallback,
                       const std::string &text,
                       const std::string &toolTip,
                       const std::string &unitsList,
                       double step,
                       double min,
                       double max,
                       double value,
                       Layout layout = LAYOUT_SLIDER_UNDER_LABEL);

    double minimum() const;
    void setMinimum(double v);

    double maximum() const;
    void setMaximum(double v);

    void setTargetProduct(double value);

    double value() const;
    void setValue(double value, bool notify = false);

    Signal<double> valueChanged;
    Signal<> finalValue;

    void slotValueChangedSlider(int v);
    void slotValueChangedSpinBox(double v);
    void slotFinalValueSlider();
    void slotFinalValueSpinBox();

private:
    Slider *slider_;
    DoubleSpinBox *spinBox_;
    double targetProduct_;

    double snapToReciprocal(double value) const;
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_SLIDER_WIDGET_HPP */
