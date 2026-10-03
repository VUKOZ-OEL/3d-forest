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

/** @file ToolBar.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <Application.hpp>
#include <ToolBar.hpp>

// Include local.
#define LOG_MODULE_NAME "ToolBar"
#include <Log.hpp>

ToolBar::ToolBar()
{
}

ToolBar::~ToolBar()
{
    // Destroy the Qt representation before deleting common children.
    destroying();

    for (const Item &item : items_)
    {
        delete item.widget;
    }
}

void ToolBar::addWidget(Widget *widget)
{
    if (!widget)
    {
        return;
    }

    if (widget == this)
    {
        throw std::invalid_argument("ToolBar cannot contain itself.");
    }

    // Prevent duplicate ownership within this toolbar.
    for (const Item &item : items_)
    {
        if (item.widget == widget)
        {
            return;
        }
    }

    const Item item{widget};

    items_.push_back(item);
    itemAdded(item);
}

void ToolBar::addSeparator()
{
    const Item item{nullptr};

    items_.push_back(item);
    itemAdded(item);
}

void ToolBar::clear()
{
    if (items_.empty())
    {
        return;
    }

    // Remove native widgets before their common data disappears.
    clearing();

    for (const Item &item : items_)
    {
        delete item.widget;
    }

    items_.clear();
}

void ToolBar::setIconSize(const Size &size)
{
    if (size.width() <= 0 || size.height() <= 0)
    {
        return;
    }

    if (iconSize_.width() == size.width() &&
        iconSize_.height() == size.height())
    {
        return;
    }

    iconSize_ = size;
    settingsChanged();
}

void ToolBar::setOrientation(int orientation)
{
    if (orientation != Ui::Horizontal && orientation != Ui::Vertical)
    {
        return;
    }

    if (orientation_ == orientation)
    {
        return;
    }

    orientation_ = orientation;
    settingsChanged();
}
