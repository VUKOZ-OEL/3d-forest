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

/** @file Splitter.cpp */

// Include std.
#include <algorithm>
#include <stdexcept>

// Include 3D Forest.
#include <Splitter.hpp>

// Include local.
#define LOG_MODULE_NAME "Splitter"
#include <Log.hpp>

Splitter::Splitter()
{
}

Splitter::~Splitter()
{
    for (Widget *widget : widgets_)
    {
        delete widget;
    }
}

void Splitter::addWidget(Widget *widget)
{
    if (!widget || widget == this)
    {
        return;
    }

    if (std::find(widgets_.begin(), widgets_.end(), widget) != widgets_.end())
    {
        return;
    }

    widgets_.push_back(widget);

    // Preserve the existing request when adding another pane.
    if (!sizes_.empty())
    {
        sizes_.push_back(1);
    }

    widgetAdded(widget);
}

int Splitter::count() const
{
    return static_cast<int>(widgets_.size());
}

Widget *Splitter::widget(int index) const
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    return widgets_[index];
}

void Splitter::setOrientation(int orientation)
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

void Splitter::setSizes(const std::vector<int> &sizes)
{
    if (sizes.size() != widgets_.size())
    {
        throw std::invalid_argument("Splitter requires one size per widget");
    }

    sizes_ = sizes;

    for (int &size : sizes_)
    {
        size = std::max(0, size);
    }

    // Emit even if the request is unchanged: the user may have
    // moved the handles since the previous call.
    sizesUpdated(sizes_);
}

void Splitter::setChildrenCollapsible(bool collapsible)
{
    if (childrenCollapsible_ == collapsible)
    {
        return;
    }

    childrenCollapsible_ = collapsible;
    settingsChanged();
}
