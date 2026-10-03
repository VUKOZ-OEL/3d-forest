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

/** @file QtIcon.hpp */

#ifndef QT_ICON_HPP
#define QT_ICON_HPP

// Include 3D Forest.
#include <Icon.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QIcon>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

EXPORT_UI_QT QPixmap toQPixmap(const Pixmap &pixmap);
EXPORT_UI_QT QIcon toQIcon(const Icon &icon);
EXPORT_UI_QT QIcon toQIcon(const ThemeIcon &icon, bool dark);

#include <WarningsEnable.hpp>

#endif /* QT_ICON_HPP */
