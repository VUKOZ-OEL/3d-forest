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

/** @file ThemeIcon.hpp */

#ifndef THEME_ICON_HPP
#define THEME_ICON_HPP

// Include std.
#include <string>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

#define THEME_ICON(name) (ThemeIcon(":/gui/", name))

/** Theme Icon. */
class EXPORT_UI_COMMON ThemeIcon
{
public:
    ThemeIcon() = default;
    ThemeIcon(const std::string &prefix, const std::string &name)
    {
        prefix_ = prefix;
        name_ = name;
    }

private:
    std::string prefix_;
    std::string name_;
};

#include <WarningsEnable.hpp>

#endif /* THEME_ICON_HPP */
