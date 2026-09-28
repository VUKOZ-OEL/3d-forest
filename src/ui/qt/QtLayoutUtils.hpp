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

/** @file QtLayoutUtils.hpp */

#ifndef QT_LAYOUT_UTILS_HPP
#define QT_LAYOUT_UTILS_HPP

// Include 3D Forest.
#include <LayoutItem.hpp>
#include <QtApplication.hpp>

// Include Qt.
#include <QBoxLayout>
#include <QLayout>
#include <QWidget>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

inline void clearQtLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0))
    {
        if (QLayout *childLayout = item->layout())
        {
            clearQtLayout(childLayout);
            delete childLayout; // The child layout is itself the item.
        }
        else
        {
            delete item->widget(); // nullptr for spacer items.
            delete item;
        }
    }
}

inline void addQtLayoutItem(QBoxLayout *layout,
                            QWidget *parent,
                            QtApplication *app,
                            const LayoutItem &item)
{
    QWidget *qtWidget;
    QLayout *qtLayout;

    switch (item.type())
    {
        case LayoutItem::WidgetItem:
            qtWidget = app->createWidget(item.widget(), parent);
            if (qtWidget)
            {
                layout->addWidget(qtWidget,
                                  item.stretch(),
                                  static_cast<Qt::Alignment>(item.alignment()));
            }
            break;

        case LayoutItem::LayoutItemType:
            qtLayout = app->createLayout(item.layout(), parent);
            if (qtLayout)
            {
                layout->addLayout(qtLayout, item.stretch());
            }
            break;

        case LayoutItem::StretchItem:
            layout->addStretch(item.stretch());
            break;

        case LayoutItem::SpacingItem:
            layout->addSpacing(item.spacing());
            break;
    }
}

#include <WarningsEnable.hpp>

#endif /* QT_LAYOUT_UTILS_HPP */
