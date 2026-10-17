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

/** @file TreeWidget.cpp */

// Include std.
#include <algorithm>
#include <functional>

// Include 3D Forest.
#include <Application.hpp>
#include <TreeWidget.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeWidget"
#include <Log.hpp>

TreeWidget::TreeWidget()
{
}

TreeWidget::~TreeWidget()
{
}

void TreeWidget::clear()
{
    // Remove Qt items before destroying the common items they reference.
    clearing();

    const bool hadSelection = !selectedItems_.empty();

    selectedItems_.clear();
    items_.clear();

    if (hadSelection)
    {
        selectionUpdated();
    }
}

TreeWidgetItem *TreeWidget::push_back(const TreeWidgetItem &item)
{
    auto copy = std::make_unique<TreeWidgetItem>(item);
    copy->attach(this, nullptr);

    TreeWidgetItem *result = copy.get();
    items_.push_back(std::move(copy));

    inserted(result);
    return result;
}

int TreeWidget::topLevelItemCount() const
{
    return static_cast<int>(items_.size());
}

TreeWidgetItem *TreeWidget::topLevelItem(int index) const
{
    if (index < 0 || index >= topLevelItemCount())
    {
        return nullptr;
    }

    return items_[index].get();
}

void TreeWidget::setColumnCount(int count)
{
    count = std::max(1, count);

    if (columnCount_ == count)
    {
        return;
    }

    columnCount_ = count;

    if (sortColumn_ >= count)
    {
        sortColumn_ = 0;
    }

    if (sortingEnabled_)
    {
        sortStorage();
    }

    settingsChanged();
}

void TreeWidget::setHeaderLabels(const std::vector<std::string> &labels)
{
    headerLabels_ = labels;
    columnCount_ = std::max(columnCount_, static_cast<int>(labels.size()));

    settingsChanged();
}

void TreeWidget::resizeColumnToContents(int column)
{
    if (column < 0 || column >= columnCount_)
    {
        return;
    }

    if (std::find(autoResizeColumns_.begin(),
                  autoResizeColumns_.end(),
                  column) == autoResizeColumns_.end())
    {
        autoResizeColumns_.push_back(column);
    }

    resizeColumnRequested(column);
}

void TreeWidget::setSelectionMode(int mode)
{
    if (mode < AbstractItemView::NoSelection ||
        mode > AbstractItemView::ContiguousSelection || mode == selectionMode_)
    {
        return;
    }

    selectionMode_ = mode;

    // Normalize existing selection for the new mode.
    setSelectedItems(selectedItems_);
    settingsChanged();
}

void TreeWidget::setSelectionBehavior(int behavior)
{
    if (behavior < AbstractItemView::SelectItems ||
        behavior > AbstractItemView::SelectColumns ||
        behavior == selectionBehavior_)
    {
        return;
    }

    selectionBehavior_ = behavior;
    settingsChanged();
}

void TreeWidget::selectRow(int row, bool notify)
{
    TreeWidgetItem *item = topLevelItem(row);

    if (item)
    {
        setSelectedItems({item}, notify);
    }
    else
    {
        setSelectedItems({}, notify);
    }
}

void TreeWidget::setSelectedItems(const std::vector<TreeWidgetItem *> &items,
                                  bool notify)
{
    std::vector<TreeWidgetItem *> selected;

    if (selectionMode_ != AbstractItemView::NoSelection)
    {
        for (TreeWidgetItem *item : items)
        {
            if (!item || item->tree_ != this)
            {
                continue;
            }

            if (std::find(selected.begin(), selected.end(), item) ==
                selected.end())
            {
                selected.push_back(item);
            }

            if (selectionMode_ == AbstractItemView::SingleSelection)
            {
                break;
            }
        }
    }

    // Selection order is not significant.
    const bool unchanged =
        selected.size() == selectedItems_.size() &&
        std::all_of(selected.begin(),
                    selected.end(),
                    [this](TreeWidgetItem *item)
                    {
                        return std::find(selectedItems_.begin(),
                                         selectedItems_.end(),
                                         item) != selectedItems_.end();
                    });

    if (unchanged)
    {
        return;
    }

    selectedItems_ = std::move(selected);
    selectionUpdated();

    if (notify && !signalsBlocked())
    {
        itemSelectionChanged();
    }
}

void TreeWidget::setSortingEnabled(bool enabled)
{
    if (sortingEnabled_ == enabled)
    {
        return;
    }

    sortingEnabled_ = enabled;

    if (enabled)
    {
        sortStorage();
    }

    settingsChanged();
}

void TreeWidget::sortItems(int column, Ui::SortOrder order)
{
    if (column < 0 || column >= columnCount_)
    {
        return;
    }

    sortColumn_ = column;
    sortOrder_ = order;

    sortStorage();
    orderUpdated();
}

void TreeWidget::sortStorage()
{
    // Use the same text comparison in the common and Qt trees.
    std::function<void(std::vector<std::unique_ptr<TreeWidgetItem>> &)>
        sortLevel;

    sortLevel = [&](auto &items)
    {
        std::stable_sort(
            items.begin(),
            items.end(),
            [this](const auto &left, const auto &right)
            {
                if (sortOrder_ == Ui::AscendingOrder)
                {
                    return left->text(sortColumn_) < right->text(sortColumn_);
                }

                return right->text(sortColumn_) < left->text(sortColumn_);
            });

        for (auto &item : items)
        {
            sortLevel(item->children_);
        }
    };

    sortLevel(items_);
}

void TreeWidget::inserted(TreeWidgetItem *item)
{
    if (sortingEnabled_)
    {
        sortStorage();
    }

    itemInserted(item);
}

void TreeWidget::itemDataChanged(TreeWidgetItem *item, int column, bool notify)
{
    if (sortingEnabled_ && column == sortColumn_)
    {
        sortStorage();
    }

    itemUpdated(item, column);

    if (notify && column >= 0 && !signalsBlocked())
    {
        itemChanged(item, column);
    }
}
