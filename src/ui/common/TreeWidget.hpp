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
    virtual ~TreeWidget();

    void clear();
    void push_back(const TreeWidgetItem &item);

    void setColumnCount(int n);

    void setHeaderLabels(const std::vector<std::string> &labels);
    void resizeColumnToContents(int col);

    void setSelectionMode(int mode);
    void setSelectionBehavior(int behavior);
    void selectRow(int row);

    void setSortingEnabled(bool b);
    void sortItems(int column, Ui::SortOrder order);

    std::vector<TreeWidgetItem *> selectedItems();
    std::vector<TreeWidgetItem> &items() { return items_; }

    Signal<TreeWidgetItem *, int> itemClicked;
    Signal<TreeWidgetItem *, int> itemChanged;
    Signal<> itemSelectionChanged;

private:
    std::vector<TreeWidgetItem> items_;
};

#include <WarningsEnable.hpp>

#endif /* TREE_WIDGET_HPP */
