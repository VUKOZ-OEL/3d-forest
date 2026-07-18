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
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ToolBar. */
class EXPORT_UI_COMMON ToolBar : public Widget
{
public:
    ToolBar();
    virtual ~ToolBar();

    void addWidget(Widget *widget);
    void addSeparator();

    void setIconSize(const Size &size);

private:
};

#include <WarningsEnable.hpp>

#endif /* TOOL_BAR_HPP */
