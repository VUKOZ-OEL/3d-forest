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

/** @file TreeWidgetItem.hpp */

#ifndef TREE_WIDGET_ITEM_HPP
#define TREE_WIDGET_ITEM_HPP

// Include std.
#include <memory>
#include <string>
#include <vector>

// Include 3D Forest.
#include <Brush.hpp>
class TreeWidget;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TreeWidgetItem. */
class EXPORT_UI_COMMON TreeWidgetItem
{
public:
    explicit TreeWidgetItem(const std::vector<std::string> &texts = {});

    TreeWidgetItem(const TreeWidgetItem &other);
    ~TreeWidgetItem();

    TreeWidgetItem &operator=(const TreeWidgetItem &) = delete;

    int columnCount() const;

    // text
    std::string text(int column) const;
    void setText(int column, const std::string &text, bool notify = false);

    // editable
    void setEditable(bool editable);
    bool isEditable() const { return editable_; }

    // check
    void setCheckState(int column, int state, bool notify = false);

    Ui::CheckState checkState(int column) const;
    bool isCheckable(int column) const;

    // selected
    void setSelected(bool selected, bool notify = false);
    bool isSelected() const;

    // background
    void setBackground(int column, const Brush &brush, bool notify = false);

    Brush background(int column) const;

    // children
    TreeWidgetItem *addChild(const TreeWidgetItem &item);

    int childCount() const;
    TreeWidgetItem *child(int index) const;
    TreeWidgetItem *parent() const { return parent_; }

    const std::vector<std::unique_ptr<TreeWidgetItem>> &children() const
    {
        return children_;
    }

private:
    friend class TreeWidget;

    void attach(TreeWidget *tree, TreeWidgetItem *parent);

    std::vector<std::string> texts_;
    std::vector<int> checkStates_; // -1 means no checkbox in that column.
    std::vector<Brush> backgrounds_;

    std::vector<std::unique_ptr<TreeWidgetItem>> children_;

    TreeWidget *tree_{nullptr};
    TreeWidgetItem *parent_{nullptr};
    bool editable_{false};
};

#include <WarningsEnable.hpp>

#endif /* TREE_WIDGET_ITEM_HPP */
