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

/** @file QtTreeWidgetItem.cpp */

// Include 3D Forest.
#include <QtTreeWidgetItem.hpp>

// Include Qt.
#include <QTreeWidget>

// Include local.
#define LOG_MODULE_NAME "QtTreeWidgetItem"
#include <Log.hpp>

QtTreeWidgetItem::QtTreeWidgetItem(TreeWidgetItem *item) : item_(item)
{
}

bool QtTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    const auto *otherItem = dynamic_cast<const QtTreeWidgetItem *>(&other);

    if (!otherItem)
    {
        return QTreeWidgetItem::operator<(other);
    }

    const int column = treeWidget() ? treeWidget()->sortColumn() : 0;

    return item_->text(column) < otherItem->item_->text(column);
}
