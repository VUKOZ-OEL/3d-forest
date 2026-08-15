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

/** @file Ui.hpp */

#ifndef UI_HPP
#define UI_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Ui. */
class EXPORT_UI_COMMON Ui
{
public:
    enum GlobalColor
    {
        color0,
        color1,
        black,
        white,
        darkGray,
        gray,
        lightGray,
        red,
        green,
        blue,
        cyan,
        magenta,
        yellow,
        darkRed,
        darkGreen,
        darkBlue,
        darkCyan,
        darkMagenta,
        darkYellow,
        transparent
    };

    enum class ColorScheme
    {
        Unknown,
        Light,
        Dark,
    };

    enum SortOrder
    {
        AscendingOrder,
        DescendingOrder
    };

    enum WindowModality
    {
        NonModal,
        WindowModal,
        ApplicationModal
    };

    enum ContextMenuPolicy
    {
        NoContextMenu,
        DefaultContextMenu,
        ActionsContextMenu,
        CustomContextMenu,
        PreventContextMenu
    };

    enum FocusPolicy
    {
        NoFocus = 0,
        TabFocus = 0x1,
        ClickFocus = 0x2,
        StrongFocus = TabFocus | ClickFocus,
        WheelFocus = StrongFocus | 0x4
    };

    enum Orientation
    {
        Horizontal = 0x1,
        Vertical = 0x2
    };

    enum CheckState
    {
        Unchecked,
        PartiallyChecked,
        Checked
    };

    enum ItemFlag
    {
        NoItemFlags = 0,
        ItemIsSelectable = 1,
        ItemIsEditable = 2,
        ItemIsDragEnabled = 4,
        ItemIsDropEnabled = 8,
        ItemIsUserCheckable = 16,
        ItemIsEnabled = 32,
        ItemIsAutoTristate = 64,
        ItemNeverHasChildren = 128,
        ItemIsUserTristate = 256
    };

    enum AlignmentFlag
    {
        AlignLeft = 0x0001,
        AlignLeading = AlignLeft,
        AlignRight = 0x0002,
        AlignTrailing = AlignRight,
        AlignHCenter = 0x0004,
        AlignJustify = 0x0008,
        AlignAbsolute = 0x0010,
        AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter |
                               AlignJustify | AlignAbsolute,

        AlignTop = 0x0020,
        AlignBottom = 0x0040,
        AlignVCenter = 0x0080,
        AlignBaseline = 0x0100,
        AlignVertical_Mask =
            AlignTop | AlignBottom | AlignVCenter | AlignBaseline,

        AlignCenter = AlignVCenter | AlignHCenter
    };

    enum DockWidgetArea
    {
        DefaultDockWidgetArea = 0,
        LeftDockWidgetArea = (1 << 0),
        RightDockWidgetArea = (1 << 1),
        TopDockWidgetArea = (1 << 1),
        BottomDockWidgetArea = (1 << 1),
    };

    enum class PanelArea
    {
        Left,
        Right,
        Top,
        Bottom,
        Main
    };

    enum BrushStyle
    {
        NoBrush,
        SolidPattern
    };
};

#include <WarningsEnable.hpp>

#endif /* UI_HPP */
