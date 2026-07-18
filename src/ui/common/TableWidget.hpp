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

/** @file TableWidget.hpp */

#ifndef TABLE_WIDGET_HPP
#define TABLE_WIDGET_HPP

// Include 3D Forest.
#include <AbstractItemView.hpp>
#include <TableWidgetItem.hpp>
#include <HeaderView.hpp>
#include <ModelIndex.hpp>
#include <Widget.hpp>
#include <ItemSelection.hpp>
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TableWidget. */
class EXPORT_UI_COMMON TableWidget : public Widget
{
public:
    TableWidget();
    virtual ~TableWidget();

    void clear();
    void setItem(int row, int col, const TableWidgetItem &item);

    void setColumnCount(int n);
    int columnCount() const { return columnCount_; }

    void setRowCount(int n);
    int rowCount() const { return rowCount_; }

    void setHeaderLabels(const std::vector<std::string> &labels);
    void resizeColumnToContents(int col);
    void setColumnWidth(int col, int width);

    HeaderView *horizontalHeader() { return &horizontalHeader_; }
    HeaderView *verticalHeader() { return &verticalHeader_; }

    void setHorizontalHeaderLabels(const std::vector<std::string> &labels);
    TableWidgetItem *horizontalHeaderItem(int col);

    void setSelectionMode(int mode);
    void setSelectionBehavior(int behavior);
    void selectRow(int row);

    ModelIndex indexAt(const Point &pos) const;

    void setSortingEnabled(bool b);
    void sortItems(int column, Ui::SortOrder order);

    void setAlternatingRowColors(bool b);

    void setContextMenuPolicy(Ui::ContextMenuPolicy contextMenuPolicy);

    std::vector<TableWidgetItem *> selectedItems();
    std::vector<TableWidgetItem> &items() { return items_; }
    TableWidgetItem *item(int row, int col);

    Signal<Point> customContextMenuRequested;
    Signal<ItemSelection, ItemSelection> selectionChanged;

    Signal<TableWidgetItem *, int> itemClicked;
    Signal<TableWidgetItem *, int> itemChanged;
    Signal<> itemSelectionChanged;

private:
    std::vector<TableWidgetItem> items_;
    int columnCount_{0};
    int rowCount_{0};
    HeaderView horizontalHeader_;
    HeaderView verticalHeader_;
};

#include <WarningsEnable.hpp>

#endif /* TABLE_WIDGET_HPP */
