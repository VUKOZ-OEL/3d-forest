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

// Include std.
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

// Include 3D Forest.
#include <AbstractItemView.hpp>
#include <HeaderView.hpp>
#include <ItemSelection.hpp>
#include <ModelIndex.hpp>
#include <Point.hpp>
#include <TableViewport.hpp>
#include <TableWidgetItem.hpp>
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TableWidget. */
class EXPORT_UI_COMMON TableWidget : public Widget
{
public:
    using Cell = std::pair<int, int>;
    using Selection = std::set<Cell>;

    TableWidget();
    ~TableWidget() override;

    TableWidget(const TableWidget &) = delete;
    TableWidget &operator=(const TableWidget &) = delete;

    void clear();

    void setItem(int row,
                 int column,
                 const TableWidgetItem &item,
                 bool notify = false);

    TableWidgetItem *item(int row, int column) const;
    std::vector<TableWidgetItem *> items() const;

    void setColumnCount(int count);
    int columnCount() const { return columnCount_; }

    void setRowCount(int count);
    int rowCount() const { return rowCount_; }

    void setHeaderLabels(const std::vector<std::string> &labels);
    void setHorizontalHeaderLabels(const std::vector<std::string> &labels);
    TableWidgetItem *horizontalHeaderItem(int column) const;

    HeaderView *horizontalHeader() { return &horizontalHeader_; }
    HeaderView *verticalHeader() { return &verticalHeader_; }

    void resizeColumnToContents(int column);
    void setColumnWidth(int column, int width);

    // Width -1 means resize to contents.
    const std::map<int, int> &columnWidths() const { return columnWidths_; }

    void setSelectionMode(int mode);
    int selectionMode() const { return selectionMode_; }

    void setSelectionBehavior(int behavior);
    int selectionBehavior() const { return selectionBehavior_; }

    void selectRow(int row, bool notify = false);
    void selectAll();
    void invertSelection();

    void clearSelection(bool notify = false);

    void setSelectedCells(const Selection &cells, bool notify = false);
    const Selection &selectedCells() const { return selection_; }

    std::set<int> selectedRows() const;
    std::vector<TableWidgetItem *> selectedItems() const;

    void setSortingEnabled(bool enabled);
    bool isSortingEnabled() const { return sortingEnabled_; }

    void sortItems(int column, Ui::SortOrder order);

    int sortColumn() const { return sortColumn_; }
    Ui::SortOrder sortOrder() const { return sortOrder_; }

    void setAlternatingRowColors(bool enabled);
    bool alternatingRowColors() const { return alternatingRowColors_; }

    void setContextMenuPolicy(Ui::ContextMenuPolicy policy);
    Ui::ContextMenuPolicy contextMenuPolicy() const
    {
        return contextMenuPolicy_;
    }

    ModelIndex indexAt(const Point &position) const;
    const TableViewport *viewport() const { return &viewport_; }

    // Installed by the backend.
    void setViewQueries(
        std::function<ModelIndex(const Point &)> indexAt,
        std::function<Point(const Point &)> mapViewportToGlobal);

    // Plugin notifications.
    Signal<Point> customContextMenuRequested;
    Signal<ItemSelection, ItemSelection> selectionChanged;

    // The second argument is the item's column.
    Signal<TableWidgetItem *, int> itemClicked;
    Signal<TableWidgetItem *, int> itemChanged;
    Signal<> itemSelectionChanged;

    // Backend notifications: emitted even when plugin signals are blocked.
    Signal<> tableReset;
    Signal<int, int> cellUpdated;
    Signal<> headersUpdated;
    Signal<> settingsChanged;
    Signal<> selectionUpdated;
    Signal<int> columnSizeRequested;
    Signal<> destroying;

private:
    friend class TableWidgetItem;

    using ItemPtr = std::unique_ptr<TableWidgetItem>;
    using Row = std::vector<ItemPtr>;

    bool validCell(int row, int column) const;
    void resizeTable(int rows, int columns);
    void attachItems();
    void itemDataChanged(TableWidgetItem *item, bool notify);
    void sortStorage();

    std::vector<Row> rows_;
    std::vector<ItemPtr> headers_;

    int columnCount_{0};
    int rowCount_{0};

    HeaderView horizontalHeader_;
    HeaderView verticalHeader_;

    std::map<int, int> columnWidths_;

    int selectionMode_{AbstractItemView::SingleSelection};
    int selectionBehavior_{AbstractItemView::SelectItems};
    Selection selection_;

    bool sortingEnabled_{false};
    int sortColumn_{0};
    Ui::SortOrder sortOrder_{Ui::AscendingOrder};

    bool alternatingRowColors_{false};
    Ui::ContextMenuPolicy contextMenuPolicy_{Ui::DefaultContextMenu};

    TableViewport viewport_;
    std::function<ModelIndex(const Point &)> indexAt_;
};

#include <WarningsEnable.hpp>

#endif /* TABLE_WIDGET_HPP */
