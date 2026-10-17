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

/** @file ToolButton.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ToolButton.hpp>

// Include local.
#define LOG_MODULE_NAME "ToolButton"
#include <Log.hpp>

ToolButton::ToolButton(const std::string &text) : text_(text)
{
}

ToolButton::~ToolButton()
{
    destroying();
}

void ToolButton::setText(const std::string &text)
{
    if (text_ == text)
    {
        return;
    }

    text_ = text;
    settingsChanged();
}

void ToolButton::setToolTip(const std::string &toolTip)
{
    if (toolTip_ == toolTip)
    {
        return;
    }

    toolTip_ = toolTip;
    settingsChanged();
}

void ToolButton::setIcon(const ThemeIcon &icon)
{
    icon_ = icon;
    settingsChanged();
}

void ToolButton::setToolButtonStyle(ToolButtonStyle style)
{
    if (style_ == style)
    {
        return;
    }

    style_ = style;
    settingsChanged();
}

void ToolButton::setAutoRaise(bool enabled)
{
    if (autoRaise_ == enabled)
    {
        return;
    }

    autoRaise_ = enabled;
    settingsChanged();
}
