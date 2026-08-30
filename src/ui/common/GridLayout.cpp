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

/** @file GridLayout.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <GridLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "GridLayout"
#include <Log.hpp>

GridLayout::GridLayout()
{
}

GridLayout::~GridLayout()
{
}

void GridLayout::clear()
{
    Layout::clear();
    positions_.clear();
}

void GridLayout::addWidget(Widget *widget, int row, int column, int alignment)
{
    addWidget(widget, row, column, 1, 1, alignment);
}

void GridLayout::addWidget(Widget *widget,
                           int row,
                           int column,
                           int rowSpan,
                           int columnSpan,
                           int alignment)
{
    if (!widget)
    {
        return;
    }

    Layout::addWidget(widget);

    positions_.push_back({row, column, rowSpan, columnSpan});
}

void GridLayout::setColumnStretch(int row, int column)
{
}
