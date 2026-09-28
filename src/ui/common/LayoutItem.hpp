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

/** @file LayoutItem.hpp */

#ifndef LAYOUT_ITEM_HPP
#define LAYOUT_ITEM_HPP

// Include 3D Forest.
class Widget;
class Layout;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** LayoutItem. */
class EXPORT_UI_COMMON LayoutItem
{
public:
    enum Type
    {
        WidgetItem,
        LayoutItemType,
        StretchItem,
        SpacingItem
    };

    explicit LayoutItem(Widget *widget, int stretch = 0, int alignment = 0);
    explicit LayoutItem(Layout *layout, int stretch = 0);

    static LayoutItem makeStretch(int stretch);
    static LayoutItem makeSpacing(int spacing);

    Type type() const { return type_; }

    Widget *widget() const { return widget_; }
    Layout *layout() const { return layout_; }

    int stretch() const { return stretch_; }
    int alignment() const { return alignment_; }
    int spacing() const { return spacing_; }

private:
    explicit LayoutItem(Type type) : type_(type) {}

    Type type_;
    Widget *widget_{nullptr};
    Layout *layout_{nullptr};

    int stretch_{0};
    int alignment_{0};
    int spacing_{0};
};

#include <WarningsEnable.hpp>

#endif /* LAYOUT_ITEM_HPP */
