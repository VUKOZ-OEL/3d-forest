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

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <RadioButton.hpp>
#include <RadioButtonGroup.hpp>

// Include local.
#define LOG_MODULE_NAME "RadioButton"
#include <Log.hpp>

RadioButtonGroup::~RadioButtonGroup()
{
    for (RadioButton *button : buttons_)
    {
        button->group_ = nullptr;
    }
}

void RadioButtonGroup::addButton(RadioButton *button)
{
    if (!button || button->group_ == this)
    {
        return;
    }

    if (button->group_)
    {
        button->group_->removeButton(button);
    }

    buttons_.push_back(button);
    button->group_ = this;

    if (button->checked_)
    {
        RadioButton *previous = checkedButton_;
        checkedButton_ = button;

        if (previous)
        {
            previous->setChecked(false);
        }
    }
}

void RadioButtonGroup::removeButton(RadioButton *button)
{
    if (!button || button->group_ != this)
    {
        return;
    }

    buttons_.erase(std::remove(buttons_.begin(), buttons_.end(), button),
                   buttons_.end());

    if (checkedButton_ == button)
    {
        checkedButton_ = nullptr;
    }

    button->group_ = nullptr;
}
