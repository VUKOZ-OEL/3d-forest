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

/** @file ToolBar.hpp */

#ifndef TOOL_BAR_HPP
#define TOOL_BAR_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ToolBar. */
class EXPORT_UI_COMMON ToolBar : public Widget
{
public:
    struct Item
    {
        // nullptr represents a separator.
        Widget *widget{nullptr};

        bool isSeparator() const { return widget == nullptr; }
    };

    ToolBar();
    ~ToolBar() override;

    ToolBar(const ToolBar &) = delete;
    ToolBar &operator=(const ToolBar &) = delete;

    // Takes ownership of the common widget.
    void addWidget(Widget *widget);
    void addSeparator();

    // Deletes owned widgets and removes separators.
    void clear();

    const std::vector<Item> &items() const { return items_; }

    void setIconSize(const Size &size);
    const Size &iconSize() const { return iconSize_; }

    void setOrientation(int orientation);
    int orientation() const { return orientation_; }

    // Backend notifications.
    Signal<Item> itemAdded;
    Signal<> clearing;
    Signal<> settingsChanged;
    Signal<> destroying;

private:
    std::vector<Item> items_;

    Size iconSize_{24, 24};
    int orientation_{Ui::Horizontal};
};

#include <WarningsEnable.hpp>

#endif /* TOOL_BAR_HPP */
