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

// Include std.
#include <algorithm>
#include <cmath>
#include <iterator>
#include <locale>
#include <numeric>
#include <sstream>
#include <stdexcept>

// Include 3D Forest.
#include <Application.hpp>
#include <TableWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "TableWidget"
#include <Log.hpp>

namespace
{

bool numericValue(const TableWidgetItem *item, long double &value)
{
    if (!item || !item->isNumeric())
    {
        return false;
    }

    std::istringstream stream(item->text());
    stream.imbue(std::locale::classic());

    if (!(stream >> value) || !std::isfinite(value))
    {
        return false;
    }

    stream >> std::ws;
    return stream.eof();
}

int compareItems(const TableWidgetItem *left, const TableWidgetItem *right)
{
    if (!left || !right)
    {
        return left ? -1 : right ? 1 : 0;
    }

    long double leftValue = 0;
    long double rightValue = 0;

    const bool leftNumeric = numericValue(left, leftValue);
    const bool rightNumeric = numericValue(right, rightValue);

    if (leftNumeric != rightNumeric)
    {
        return leftNumeric ? -1 : 1;
    }

    if (leftNumeric)
    {
        return leftValue < rightValue ? -1 : leftValue > rightValue ? 1 : 0;
    }

    return left->text().compare(right->text());
}

ItemSelection difference(const TableWidget::Selection &left,
                         const TableWidget::Selection &right)
{
    std::vector<ModelIndex> indexes;

    for (const auto &cell : left)
    {
        if (right.count(cell) == 0)
        {
            indexes.emplace_back(cell.first, cell.second);
        }
    }

    return ItemSelection(std::move(indexes));
}

} // namespace

TableWidget::TableWidget() = default;

TableWidget::~TableWidget()
{
    destroying();
}

bool TableWidget::validCell(int row, int column) const
{
    return row >= 0 && row < rowCount_ && column >= 0 && column < columnCount_;
}

TableWidgetItem *TableWidget::item(int row, int column) const
{
    return validCell(row, column) ? rows_[row][column].get() : nullptr;
}

std::vector<TableWidgetItem *> TableWidget::items() const
{
    std::vector<TableWidgetItem *> result;

    for (const auto &row : rows_)
    {
        for (const auto &item : row)
        {
            if (item)
            {
                result.push_back(item.get());
            }
        }
    }

    return result;
}

void TableWidget::attachItems()
{
    for (int row = 0; row < rowCount_; ++row)
    {
        for (int column = 0; column < columnCount_; ++column)
        {
            if (auto *value = item(row, column))
            {
                value->table_ = this;
                value->row_ = row;
                value->column_ = column;
            }
        }
    }

    for (int column = 0; column < columnCount_; ++column)
    {
        if (auto *value = headers_[column].get())
        {
            value->table_ = this;
            value->row_ = -1;
            value->column_ = column;
        }
    }
}

void TableWidget::resizeTable(int rows, int columns)
{
    rows = (std::max)(0, rows);
    columns = (std::max)(0, columns);

    if (rows == rowCount_ && columns == columnCount_)
    {
        return;
    }

    rows_.resize(rows);

    for (auto &row : rows_)
    {
        row.resize(columns);
    }

    headers_.resize(columns);
    rowCount_ = rows;
    columnCount_ = columns;

    for (auto it = selection_.begin(); it != selection_.end();)
    {
        if (!validCell(it->first, it->second))
        {
            it = selection_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = columnWidths_.begin(); it != columnWidths_.end();)
    {
        if (it->first >= columnCount_)
        {
            it = columnWidths_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (sortColumn_ >= columnCount_)
    {
        sortColumn_ = 0;
    }

    attachItems();

    if (sortingEnabled_)
    {
        sortStorage();
    }

    tableReset();
}

void TableWidget::setColumnCount(int count)
{
    resizeTable(rowCount_, count);
}

void TableWidget::setRowCount(int count)
{
    resizeTable(count, columnCount_);
}

void TableWidget::clear()
{
    for (auto &row : rows_)
    {
        for (auto &value : row)
        {
            value.reset();
        }
    }

    for (auto &header : headers_)
    {
        header.reset();
    }

    selection_.clear();
    tableReset();
}

void TableWidget::setItem(int row,
                          int column,
                          const TableWidgetItem &source,
                          bool notify)
{
    if (!validCell(row, column))
    {
        throw std::out_of_range("TableWidget::setItem");
    }

    // Copy first: source may be the item being replaced.
    auto value = std::make_unique<TableWidgetItem>(source);
    value->table_ = this;
    value->row_ = row;
    value->column_ = column;

    TableWidgetItem *stored = value.get();
    rows_[row][column] = std::move(value);

    itemDataChanged(stored, notify);
}

void TableWidget::itemDataChanged(TableWidgetItem *value, bool notify)
{
    if (value->row_ < 0)
    {
        headersUpdated();
        return;
    }

    if (sortingEnabled_ && value->column_ == sortColumn_)
    {
        sortStorage();
        tableReset();
    }
    else
    {
        cellUpdated(value->row_, value->column_);
    }

    if (notify && !signalsBlocked())
    {
        itemChanged(value, value->column_);
    }
}

void TableWidget::setHeaderLabels(const std::vector<std::string> &labels)
{
    setHorizontalHeaderLabels(labels);
}

void TableWidget::setHorizontalHeaderLabels(
    const std::vector<std::string> &labels)
{
    const int count = (std::min)(columnCount_, static_cast<int>(labels.size()));

    for (int column = 0; column < count; ++column)
    {
        headers_[column] = std::make_unique<TableWidgetItem>(labels[column]);

        headers_[column]->table_ = this;
        headers_[column]->column_ = column;
    }

    headersUpdated();
}

TableWidgetItem *TableWidget::horizontalHeaderItem(int column) const
{
    return column >= 0 && column < columnCount_ ? headers_[column].get()
                                                : nullptr;
}

void TableWidget::resizeColumnToContents(int column)
{
    if (column < 0 || column >= columnCount_)
    {
        return;
    }

    columnWidths_[column] = -1;
    columnSizeRequested(column);
}

void TableWidget::setColumnWidth(int column, int width)
{
    if (column < 0 || column >= columnCount_ || width < 0)
    {
        return;
    }

    columnWidths_[column] = width;
    columnSizeRequested(column);
}

void TableWidget::setSelectionMode(int mode)
{
    if (mode < AbstractItemView::NoSelection ||
        mode > AbstractItemView::ContiguousSelection || selectionMode_ == mode)
    {
        return;
    }

    selectionMode_ = mode;
    clearSelection();
    settingsChanged();
}

void TableWidget::setSelectionBehavior(int behavior)
{
    if (behavior < AbstractItemView::SelectItems ||
        behavior > AbstractItemView::SelectColumns ||
        selectionBehavior_ == behavior)
    {
        return;
    }

    selectionBehavior_ = behavior;
    clearSelection();
    settingsChanged();
}

void TableWidget::setSelectedCells(const Selection &cells, bool notify)
{
    Selection next;

    if (selectionMode_ != AbstractItemView::NoSelection)
    {
        for (const auto &cell : cells)
        {
            if (validCell(cell.first, cell.second))
            {
                next.insert(cell);
            }
        }
    }

    if (next == selection_)
    {
        return;
    }

    const ItemSelection selected = difference(next, selection_);
    const ItemSelection deselected = difference(selection_, next);

    selection_ = std::move(next);

    // The backend must still update while plugin signals are blocked.
    selectionUpdated();

    if (notify && !signalsBlocked())
    {
        selectionChanged(selected, deselected);
        itemSelectionChanged();
    }
}

void TableWidget::clearSelection(bool notify)
{
    setSelectedCells({}, notify);
}

void TableWidget::selectRow(int row, bool notify)
{
    if (row < 0 || row >= rowCount_ ||
        selectionMode_ == AbstractItemView::NoSelection)
    {
        return;
    }

    Selection next;

    // Repeated selectRow() calls accumulate in multi-selection modes.
    if (selectionMode_ != AbstractItemView::SingleSelection)
    {
        next = selection_;
    }

    for (int column = 0; column < columnCount_; ++column)
    {
        next.emplace(row, column);
    }

    setSelectedCells(next, notify);
}

std::set<int> TableWidget::selectedRows() const
{
    std::map<int, int> counts;

    for (const auto &cell : selection_)
    {
        ++counts[cell.first];
    }

    std::set<int> result;

    for (const auto &entry : counts)
    {
        if (columnCount_ > 0 && entry.second == columnCount_)
        {
            result.insert(entry.first);
        }
    }

    return result;
}

std::vector<TableWidgetItem *> TableWidget::selectedItems() const
{
    std::vector<TableWidgetItem *> result;

    for (const auto &cell : selection_)
    {
        if (auto *value = item(cell.first, cell.second))
        {
            result.push_back(value);
        }
    }

    return result;
}

void TableWidget::sortStorage()
{
    if (sortColumn_ < 0 || sortColumn_ >= columnCount_)
    {
        return;
    }

    std::vector<int> order(rowCount_);
    std::iota(order.begin(), order.end(), 0);

    std::stable_sort(
        order.begin(),
        order.end(),
        [this](int left, int right)
        {
            const int comparison =
                compareItems(item(left, sortColumn_), item(right, sortColumn_));

            return sortOrder_ == Ui::AscendingOrder ? comparison < 0
                                                    : comparison > 0;
        });

    std::vector<int> newRow(rowCount_);
    std::vector<Row> sorted;
    sorted.reserve(rows_.size());

    for (int row = 0; row < rowCount_; ++row)
    {
        newRow[order[row]] = row;
        sorted.push_back(std::move(rows_[order[row]]));
    }

    rows_ = std::move(sorted);

    Selection selected;

    for (const auto &cell : selection_)
    {
        selected.emplace(newRow[cell.first], cell.second);
    }

    selection_ = std::move(selected);
    attachItems();
}

void TableWidget::setSortingEnabled(bool enabled)
{
    if (sortingEnabled_ == enabled)
    {
        return;
    }

    sortingEnabled_ = enabled;

    if (enabled)
    {
        sortStorage();
        tableReset();
    }

    settingsChanged();
}

void TableWidget::sortItems(int column, Ui::SortOrder order)
{
    if (column < 0 || column >= columnCount_)
    {
        return;
    }

    sortColumn_ = column;
    sortOrder_ = order;

    sortStorage();
    tableReset();
    settingsChanged();
}

void TableWidget::setAlternatingRowColors(bool enabled)
{
    if (alternatingRowColors_ == enabled)
    {
        return;
    }

    alternatingRowColors_ = enabled;
    settingsChanged();
}

void TableWidget::setContextMenuPolicy(Ui::ContextMenuPolicy policy)
{
    if (contextMenuPolicy_ == policy)
    {
        return;
    }

    contextMenuPolicy_ = policy;
    settingsChanged();
}

ModelIndex TableWidget::indexAt(const Point &position) const
{
    return indexAt_ ? indexAt_(position) : ModelIndex();
}

void TableWidget::setViewQueries(
    std::function<ModelIndex(const Point &)> indexAt,
    std::function<Point(const Point &)> mapViewportToGlobal)
{
    indexAt_ = std::move(indexAt);
    viewport_.mapToGlobal_ = std::move(mapViewportToGlobal);
}
