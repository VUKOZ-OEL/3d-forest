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

/** @file IconTheme.hpp */

#ifndef ICON_THEME_HPP
#define ICON_THEME_HPP

#include <ExportGui.hpp>

#include <QIcon>
#include <QString>

/** Icon Theme.

    QIcon encapsulates multiple pixel perfect icon sizes.
    Qt will automatically select the best size for a given rendering.
*/
class EXPORT_GUI IconTheme : public QIcon
{
public:
    IconTheme(const QString &prefix, const QString &name) : QIcon()
    {
        addFile(prefix + name + "_24px.png", QSize(24, 24));
        addFile(prefix + name + "_24px.svg", QSize(24, 24));
        addFile(prefix + name + "_30px.svg", QSize(30, 30));
    }
};

#endif /* ICON_THEME_HPP */
