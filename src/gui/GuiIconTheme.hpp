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

/** @file GuiIconTheme.hpp */

#ifndef GUI_ICON_THEME_HPP
#define GUI_ICON_THEME_HPP

#include <QIcon>
#include <QString>

/** Gui Icon Theme.

    QIcon encapsulates multiple pixel perfect icon sizes.
    Qt will automatically select the best size for a given rendering.
*/
class GuiIconTheme : public QIcon
{
public:
    GuiIconTheme(const QString &prefix, const QString &name) : QIcon()
    {
        addFile(prefix + name + "-regular-24.png", QSize(24, 24));
    }
};

#endif /* GUI_ICON_THEME_HPP */
