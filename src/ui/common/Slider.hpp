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

/** @file Slider.hpp */

#ifndef SLIDER_HPP
#define SLIDER_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Slider. */
class EXPORT_UI_COMMON Slider : public Widget
{
public:
    enum TickPosition
    {
        NoTicks = 0,
        TicksAbove = 1,
        TicksLeft = TicksAbove,
        TicksBelow = 2,
        TicksRight = TicksBelow,
        TicksBothSides = 3
    };

    Slider();
    virtual ~Slider();

    int singleStep() const { return singleStep_; }
    void setSingleStep(int val);

    void setTickInterval(int v);
    void setTickPosition(int v);
    void setOrientation(int v);

    int minimum() const { return minimum_; }
    void setMinimum(int min);

    int maximum() const { return maximum_; }
    void setMaximum(int max);

    void setRange(int min, int max);

    int value() const { return value_; }
    void setValue(int value, bool notify = false);

    Signal<int> valueChanged;
    Signal<> sliderReleased;

private:
    int singleStep_{0};
    int minimum_{0};
    int maximum_{0};
    int value_{0};
};

#include <WarningsEnable.hpp>

#endif /* SLIDER_HPP */
