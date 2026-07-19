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

/** @file TableWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <TableWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "TableWidget"
#include <Log.hpp>

TableWidget::TableWidget()
{
}

TableWidget::~TableWidget()
{
}

void TableWidget::clear()
{
}

void TableWidget::setItem(int row, int col, const TableWidgetItem &item)
{
    items_.push_back(item);
}

void TableWidget::setColumnCount(int n)
{
}

void TableWidget::setRowCount(int n)
{
}

void TableWidget::setHeaderLabels(const std::vector<std::string> &labels)
{
}

void TableWidget::resizeColumnToContents(int col)
{
}

void TableWidget::setColumnWidth(int col, int width)
{
}

void TableWidget::setHorizontalHeaderLabels(
    const std::vector<std::string> &labels)
{
}

TableWidgetItem *TableWidget::horizontalHeaderItem(int col)
{
    return nullptr;
}

void TableWidget::setSelectionMode(int mode)
{
}

void TableWidget::setSelectionBehavior(int behavior)
{
}

void TableWidget::selectRow(int row)
{
}

std::set<int> TableWidget::selectedRows() const
{
#if 0
    QModelIndexList indexes = tableWidget_->selectionModel()->selectedIndexes();
    std::set<int> selectedRows;
    for (const ModelIndex &index : indexes)
    {
        selectedRows.insert(index.row());
    }
#endif

    std::set<int> list;
    return list;
}

ModelIndex TableWidget::indexAt(const Point &pos) const
{
    ModelIndex index;
    return index;
}

const Widget *TableWidget::viewport() const
{
    return this;
}

void TableWidget::setSortingEnabled(bool b)
{
}

void TableWidget::sortItems(int column, Ui::SortOrder order)
{
}

void TableWidget::setAlternatingRowColors(bool b)
{
}

void TableWidget::setContextMenuPolicy(Ui::ContextMenuPolicy contextMenuPolicy)
{
}

std::vector<TableWidgetItem *> TableWidget::selectedItems()
{
    std::vector<TableWidgetItem *> list;
    return list;
}

TableWidgetItem *TableWidget::item(int row, int col)
{
    return nullptr;
}
