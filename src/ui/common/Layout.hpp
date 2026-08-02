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

    void clear();

    void addWidget(Widget *widget, int stretch = 0);
    void addWidget(Splitter *widget, int stretch = 0);
    void addLayout(Layout *layout, int stretch = 0);
    void addStretch();
    void addSpacing(int spacing);

    void setContentsMargins(int left, int top, int right, int bottom);

    void addWidget(Widget *widget, int row, int column, int alignment = 0);
    void addWidget(Widget *widget,
                   int row,
                   int column,
                   int rowSpan,
                   int columnSpan,
                   int alignment = 0);

    const std::vector<Widget *> &widgets() const
    {
        return widgets_;
    }

    Signal<Widget *> widgetAdded;

private:
    std::vector<Widget *> widgets_;
};

#include <WarningsEnable.hpp>

#endif /* LAYOUT_HPP */
