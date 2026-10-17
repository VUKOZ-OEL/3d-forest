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

/** @file RadioButton.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <RadioButton.hpp>
#include <RadioButtonGroup.hpp>

// Include local.
#define LOG_MODULE_NAME "RadioButton"
#include <Log.hpp>

RadioButton::RadioButton(const std::string &str) : text_(str)
{
}

RadioButton::~RadioButton()
{
    if (group_)
    {
        group_->removeButton(this);
    }
}

void RadioButton::setText(const std::string &text)
{
    if (text_ == text)
    {
        return;
    }

    text_ = text;
    textChanged(text_);
}

void RadioButton::setChecked(bool checked, bool notify)
{
    if (checked_ == checked)
    {
        return;
    }

    checked_ = checked;

    if (group_)
    {
        if (checked)
        {
            RadioButton *previous = group_->checkedButton_;
            group_->checkedButton_ = this;

            if (previous && previous != this)
            {
                previous->setChecked(false, notify);
            }
        }
        else if (group_->checkedButton_ == this)
        {
            group_->checkedButton_ = nullptr;
        }
    }

    checkedUpdated(checked_);

    if (notify && !signalsBlocked())
    {
        stateChanged(checked_ ? 1 : 0);
    }
}

void RadioButton::setGroup(RadioButtonGroup *group)
{
    if (group_ == group)
    {
        return;
    }

    if (group_)
    {
        group_->removeButton(this);
    }

    if (group)
    {
        group->addButton(this);
    }
}
