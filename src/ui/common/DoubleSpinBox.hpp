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

/** @file DoubleSpinBox.hpp */

#ifndef DOUBLE_SPIN_BOX_HPP
#define DOUBLE_SPIN_BOX_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** DoubleSpinBox. */
class EXPORT_UI_COMMON DoubleSpinBox : public Widget
{
public:
    DoubleSpinBox();
    virtual ~DoubleSpinBox();

    double singleStep() const { return singleStep_; }
    void setSingleStep(double val);

    double minimum() const { return minimum_; }
    void setMinimum(double min);

    double maximum() const { return maximum_; }
    void setMaximum(double max);

    void setRange(double min, double max);

    double value() const { return value_; }
    void setValue(double value, bool notify = false);

    Signal<double> valueChanged;
    Signal<> editingFinished;

private:
    double singleStep_{0.0};
    double minimum_{0.0};
    double maximum_{0.0};
    double value_{0.0};
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_SPIN_BOX_HPP */
