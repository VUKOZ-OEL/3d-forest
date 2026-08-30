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

/** @file QtGridLayout.hpp */

#ifndef QT_GRID_LAYOUT_HPP
#define QT_GRID_LAYOUT_HPP

// Include 3D Forest.
#include <GridLayout.hpp>
class QtApplication;

// Include Qt.
#include <QGridLayout>
class QWidget;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtGridLayout. */
class EXPORT_UI_QT QtGridLayout : public QGridLayout
{
public:
    explicit QtGridLayout(GridLayout *layout,
                          QtApplication *app,
                          QWidget *parent = nullptr);

private:
    GridLayout *layout_;
};

#include <WarningsEnable.hpp>

#endif /* QT_GRID_LAYOUT_HPP */
