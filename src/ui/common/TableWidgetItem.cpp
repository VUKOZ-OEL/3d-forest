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

/** @file TableWidgetItem.cpp */

// Include 3D Forest.
#include <TableWidget.hpp>
#include <TableWidgetItem.hpp>

// Include local.
#define LOG_MODULE_NAME "TableWidgetItem"
#include <Log.hpp>

TableWidgetItem::TableWidgetItem(const std::string &text) : text_(text)
{
}

TableWidgetItem::TableWidgetItem(const TableWidgetItem &other)
    : text_(other.text_),
      numeric_(other.numeric_),
      flags_(other.flags_),
      checkState_(other.checkState_),
      background_(other.background_)
{
}

void TableWidgetItem::changed(bool notify)
{
    if (table_)
    {
        table_->itemDataChanged(this, notify);
    }
}

void TableWidgetItem::setText(const std::string &text, bool notify)
{
    if (text_ == text)
    {
        return;
    }

    text_ = text;
    changed(notify);
}

void TableWidgetItem::setNumeric(bool numeric)
{
    if (numeric_ == numeric)
    {
        return;
    }

    numeric_ = numeric;
    changed(false);
}

void TableWidgetItem::setFlags(int flags)
{
    if (flags_ == flags)
    {
        return;
    }

    flags_ = flags;
    changed(false);
}

Ui::CheckState TableWidgetItem::checkState() const
{
    return checkState_ < 0 ? Ui::Unchecked
                           : static_cast<Ui::CheckState>(checkState_);
}

void TableWidgetItem::setCheckState(Ui::CheckState state, bool notify)
{
    const int value = static_cast<int>(state);

    if (value < 0 || value > 2 || checkState_ == value)
    {
        return;
    }

    checkState_ = value;
    changed(notify);
}

void TableWidgetItem::setBackground(const Brush &background)
{
    background_ = background;
    changed(false);
}
