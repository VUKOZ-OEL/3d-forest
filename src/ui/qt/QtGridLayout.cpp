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

/** @file QtGridLayout.cpp */

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtGridLayout.hpp>

// Include Qt.
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtGridLayout"
#include <Log.hpp>

QtGridLayout::QtGridLayout(GridLayout *layout,
                           QtApplication *app,
                           QWidget *parent)
    : QGridLayout(),
      layout_(layout)
{
    for (std::size_t i = 0; i < layout_->count(); ++i)
    {
        const LayoutItem &item = layout_->itemAt(i);
        const GridLayout::Position &position = layout_->positionAt(i);

        if (item.widget())
        {
            QWidget *qtWidget = app->createWidget(item.widget(), parent);

            QGridLayout::addWidget(qtWidget,
                                   position.row,
                                   position.column,
                                   position.rowSpan,
                                   position.columnSpan);
        }
        else if (item.layout())
        {
            QLayout *qtLayout = app->createLayout(item.layout(), parent);

            QGridLayout::addLayout(qtLayout,
                                   position.row,
                                   position.column,
                                   position.rowSpan,
                                   position.columnSpan);
        }
    }
}
