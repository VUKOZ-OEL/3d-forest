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

/** @file TreeWidget.hpp */

#ifndef TREE_WIDGET_HPP
#define TREE_WIDGET_HPP

// Include std.
#include <memory>
#include <string>
#include <vector>

// Include 3D Forest.
#include <AbstractItemView.hpp>
#include <TreeWidgetItem.hpp>
#include <Widget.hpp>
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TreeWidget. */
class EXPORT_UI_COMMON TreeWidget : public Widget
{
public:
    TreeWidget();
    ~TreeWidget() override;

    void clear();
    TreeWidgetItem *push_back(const TreeWidgetItem &item);

    int topLevelItemCount() const;
    TreeWidgetItem *topLevelItem(int index) const;

    const std::vector<std::unique_ptr<TreeWidgetItem>> &items() const
    {
        return items_;
    }

    void setColumnCount(int count);
    int columnCount() const { return columnCount_; }

    void setHeaderLabels(const std::vector<std::string> &labels);
    const std::vector<std::string> &headerLabels() const
    {
        return headerLabels_;
    }

    void resizeColumnToContents(int column);
    const std::vector<int> &autoResizeColumns() const
    {
        return autoResizeColumns_;
    }

    void setSelectionMode(int mode);
    int selectionMode() const { return selectionMode_; }

    void setSelectionBehavior(int behavior);
    int selectionBehavior() const { return selectionBehavior_; }

    // Index in the common tree's current top-level order.
    void selectRow(int row, bool notify = false);

    std::vector<TreeWidgetItem *> selectedItems() const
    {
        return selectedItems_;
    }

    void setSelectedItems(const std::vector<TreeWidgetItem *> &items,
                          bool notify = false);

    void setSortingEnabled(bool enabled);
    bool isSortingEnabled() const { return sortingEnabled_; }

    void sortItems(int column, Ui::SortOrder order);
    int sortColumn() const { return sortColumn_; }
    Ui::SortOrder sortOrder() const { return sortOrder_; }

    // Plugin notifications.
    Signal<TreeWidgetItem *, int> itemClicked;
    Signal<TreeWidgetItem *, int> itemChanged;
    Signal<> itemSelectionChanged;

    // Backend notifications.
    Signal<> settingsChanged;
    Signal<> clearing;
    Signal<TreeWidgetItem *> itemInserted;
    Signal<TreeWidgetItem *, int> itemUpdated;
    Signal<> selectionUpdated;
    Signal<> orderUpdated;
    Signal<int> resizeColumnRequested;

private:
    friend class TreeWidgetItem;

    void itemDataChanged(TreeWidgetItem *item, int column, bool notify);
    void inserted(TreeWidgetItem *item);
    void sortStorage();

    std::vector<std::unique_ptr<TreeWidgetItem>> items_;
    std::vector<TreeWidgetItem *> selectedItems_;

    std::vector<std::string> headerLabels_;
    std::vector<int> autoResizeColumns_;

    int columnCount_{1};
    int selectionMode_{AbstractItemView::SingleSelection};
    int selectionBehavior_{AbstractItemView::SelectRows};

    bool sortingEnabled_{false};
    int sortColumn_{0};
    Ui::SortOrder sortOrder_{Ui::AscendingOrder};
};

#include <WarningsEnable.hpp>

#endif /* TREE_WIDGET_HPP */
