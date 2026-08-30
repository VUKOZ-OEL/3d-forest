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

/** @file Layout.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <Layout.hpp>
#include <Splitter.hpp>

// Include local.
#define LOG_MODULE_NAME "Layout"
#include <Log.hpp>

Layout::Layout()
{
}

Layout::~Layout()
{
    clear();
}

void Layout::clear()
{
    for (const LayoutItem &item : items_)
    {
        if (item.widget())
        {
            delete item.widget();
        }
        else if (item.layout())
        {
            delete item.layout();
        }
    }

    items_.clear();
}

void Layout::addWidget(Widget *widget, int stretch)
{
    if (!widget)
    {
        return;
    }

    items_.push_back(LayoutItem(widget));
}

void Layout::addLayout(Layout *layout, int stretch)
{
    if (!layout)
    {
        return;
    }

    items_.push_back(LayoutItem(layout));
}

void Layout::addStretch()
{
}

void Layout::addSpacing(int spacing)
{
}

void Layout::setContentsMargins(int left, int top, int right, int bottom)
{
}
