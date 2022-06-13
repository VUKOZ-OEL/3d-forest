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

#include <ExportGui.hpp>

#include <QIcon>
#include <QString>

#define THEME_ICON(name) (ThemeIcon(":/gui/", name))

/** Theme Icon.

    QIcon encapsulates multiple pixel perfect icon sizes.
    Qt will automatically select the best size for a given rendering.
*/
class EXPORT_GUI ThemeIcon : public QIcon
{
public:
    ThemeIcon(const QString &prefix, const QString &name) : QIcon()
    {
        addFile(prefix + name + "_16px.png", QSize(16, 16));
        addFile(prefix + name + "_24px.png", QSize(24, 24));
        addFile(prefix + name + "_30px.png", QSize(30, 30));
    }
};

#endif /* THEME_ICON_HPP */