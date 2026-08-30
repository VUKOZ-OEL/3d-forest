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

/** @file ComboBox.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <Application.hpp>
#include <ComboBox.hpp>

// Include local.
#define LOG_MODULE_NAME "ComboBox"
#include <Log.hpp>

ComboBox::ComboBox()
{
}

ComboBox::~ComboBox()
{
}

void ComboBox::addItem(const std::string &str)
{
    items_.push_back(str);
    itemAdded(items_.back());

    // Like QComboBox, adding the first item selects it.
    if (value_ == -1)
    {
        value_ = 0;
        currentIndexChanged(value_);
    }
}

std::string ComboBox::itemText(int index) const
{
    if (index < 0 || index >= count())
    {
        return {};
    }

    return items_[static_cast<std::size_t>(index)];
}

void ComboBox::setCurrentText(const std::string &str, bool notify)
{
    for (int i = 0; i < count(); ++i)
    {
        if (items_[static_cast<std::size_t>(i)] == str)
        {
            setValue(i, notify);
            return;
        }
    }
}

std::string ComboBox::currentText() const
{
    return itemText(value_);
}

int ComboBox::currentIndex() const
{
    return value_;
}

void ComboBox::setValue(int value, bool notify)
{
    if (value < -1 || value >= count())
    {
        return;
    }

    const bool changed = value_ != value;

    if (changed)
    {
        value_ = value;
        currentIndexChanged(value_);
    }

    // QComboBox::activated may occur even when the same item
    // is selected again.
    if (notify && !signalsBlocked())
    {
        activated(value_);
    }
}
