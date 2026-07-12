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

/** @file SpinBox.hpp */

#ifndef SPIN_BOX_HPP
#define SPIN_BOX_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** SpinBox. */
class EXPORT_UI_COMMON SpinBox : public Widget
{
public:
    SpinBox();
    virtual ~SpinBox();

    int singleStep() const { return singleStep_; }
    void setSingleStep(int val);

    int minimum() const { return minimum_; }
    void setMinimum(int min);

    int maximum() const { return maximum_; }
    void setMaximum(int max);

    void setRange(int min, int max);

    int value() const { return value_; }
    void setValue(int value, bool notify = false);

    Signal<int> valueChanged;
    Signal<> editingFinished;

private:
    int singleStep_{0.0};
    int minimum_{0.0};
    int maximum_{0.0};
    int value_{0.0};
};

#include <WarningsEnable.hpp>

#endif /* SPIN_BOX_HPP */
