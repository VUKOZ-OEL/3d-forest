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

/** @file Layout.hpp */

#ifndef LAYOUT_HPP
#define LAYOUT_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <LayoutItem.hpp>
#include <Signal.hpp>
class Application;
class Widget;
class Splitter;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Layout. */
class EXPORT_UI_COMMON Layout
{
public:
    Layout();
    virtual ~Layout();

    Layout(const Layout &) = delete;
    Layout &operator=(const Layout &) = delete;

    virtual void clear();

    void addWidget(Widget *widget, int stretch = 0);
    void addLayout(Layout *layout, int stretch = 0);
    void addStretch();
    void addSpacing(int spacing);

    void setContentsMargins(int left, int top, int right, int bottom);

    // const std::vector<Widget *> &widgets() const { return widgets_; }
    std::size_t count() const { return items_.size(); }

    const LayoutItem &itemAt(std::size_t index) const
    {
        return items_.at(index);
    }

    const std::vector<LayoutItem> &items() const { return items_; }

    Signal<Widget *> widgetAdded;

private:
    std::vector<LayoutItem> items_;
};

#include <WarningsEnable.hpp>

#endif /* LAYOUT_HPP */
