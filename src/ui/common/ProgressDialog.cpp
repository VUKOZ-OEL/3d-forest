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

/** @file ProgressDialog.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <ProgressDialog.hpp>

// Include local.
#define LOG_MODULE_NAME "ProgressDialog"
#include <Log.hpp>

ProgressDialog::ProgressDialog(Application *app) : Dialog(app)
{
    setStandardButtons(NoButton);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    if (minimum < 0 || maximum < minimum)
    {
        throw std::invalid_argument("Invalid progress range");
    }

    if (minimum_ == minimum && maximum_ == maximum)
    {
        return;
    }

    minimum_ = minimum;
    maximum_ = maximum;

    if (value_ < minimum_ || value_ > maximum_)
    {
        value_ = minimum_ - 1;
    }

    rangeChanged(minimum_, maximum_);
    valueChanged(value_);
}

void ProgressDialog::setValue(int value)
{
    if (value < minimum_ || value > maximum_)
    {
        return;
    }

    if (value_ == value)
    {
        return;
    }

    value_ = value;
    valueChanged(value_);
}

void ProgressDialog::setLabelText(const std::string &text)
{
    if (labelText_ == text)
    {
        return;
    }

    labelText_ = text;
    labelTextChanged(labelText_);
}

void ProgressDialog::setCancelButtonText(const std::string &text)
{
    if (cancelButtonText_ == text)
    {
        return;
    }

    cancelButtonText_ = text;
    cancelButtonTextChanged(cancelButtonText_);
}

void ProgressDialog::cancel()
{
    if (canceled_)
    {
        return;
    }

    canceled_ = true;
    hide();
    canceled();
}

void ProgressDialog::reset()
{
    canceled_ = false;
    value_ = minimum_ - 1;

    resetRequested();
}
