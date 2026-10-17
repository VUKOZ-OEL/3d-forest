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

/** @file TreeWidgetItem.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <TreeWidget.hpp>
#include <TreeWidgetItem.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeWidgetItem"
#include <Log.hpp>

TreeWidgetItem::TreeWidgetItem(const std::vector<std::string> &texts)
    : texts_(texts)
{
}

TreeWidgetItem::TreeWidgetItem(const TreeWidgetItem &other)
    : texts_(other.texts_),
      editable_(other.editable_),
      checkStates_(other.checkStates_),
      backgrounds_(other.backgrounds_)
{
    for (const auto &child : other.children_)
    {
        auto copy = std::make_unique<TreeWidgetItem>(*child);
        copy->attach(nullptr, this);
        children_.push_back(std::move(copy));
    }
}

TreeWidgetItem::~TreeWidgetItem() = default;

int TreeWidgetItem::columnCount() const
{
    return static_cast<int>(
        std::max({texts_.size(), checkStates_.size(), backgrounds_.size()}));
}

std::string TreeWidgetItem::text(int column) const
{
    if (column < 0 || column >= static_cast<int>(texts_.size()))
    {
        return {};
    }

    return texts_[column];
}

void TreeWidgetItem::setText(int column, const std::string &text, bool notify)
{
    if (column < 0)
    {
        return;
    }

    if (column >= static_cast<int>(texts_.size()))
    {
        texts_.resize(static_cast<size_t>(column) + 1);
    }

    if (texts_[column] == text)
    {
        return;
    }

    texts_[column] = text;

    if (tree_)
    {
        tree_->itemDataChanged(this, column, notify);
    }
}

void TreeWidgetItem::setEditable(bool editable)
{
    if (editable_ == editable)
    {
        return;
    }

    editable_ = editable;

    if (tree_)
    {
        tree_->itemDataChanged(this, -1, false);
    }
}

bool TreeWidgetItem::isCheckable(int column) const
{
    return column >= 0 && column < static_cast<int>(checkStates_.size()) &&
           checkStates_[column] >= 0;
}

Ui::CheckState TreeWidgetItem::checkState(int column) const
{
    if (!isCheckable(column))
    {
        return Ui::Unchecked;
    }

    return static_cast<Ui::CheckState>(checkStates_[column]);
}

void TreeWidgetItem::setCheckState(int column, int state, bool notify)
{
    if (column < 0 || state < Ui::Unchecked || state > Ui::Checked)
    {
        return;
    }

    if (column >= static_cast<int>(checkStates_.size()))
    {
        checkStates_.resize(static_cast<size_t>(column) + 1, -1);
    }

    if (checkStates_[column] == state)
    {
        return;
    }

    checkStates_[column] = state;

    if (tree_)
    {
        tree_->itemDataChanged(this, column, notify);
    }
}

bool TreeWidgetItem::isSelected() const
{
    if (!tree_)
    {
        return false;
    }

    const auto selected = tree_->selectedItems();

    return std::find(selected.begin(), selected.end(), this) != selected.end();
}

void TreeWidgetItem::setSelected(bool selected, bool notify)
{
    if (!tree_)
    {
        return;
    }

    auto items = tree_->selectedItems();
    const auto found = std::find(items.begin(), items.end(), this);

    if (selected)
    {
        if (found != items.end())
        {
            return;
        }

        if (tree_->selectionMode() == AbstractItemView::SingleSelection)
        {
            items.clear();
        }

        items.push_back(this);
    }
    else
    {
        if (found == items.end())
        {
            return;
        }

        items.erase(found);
    }

    tree_->setSelectedItems(items, notify);
}

Brush TreeWidgetItem::background(int column) const
{
    if (column < 0 || column >= static_cast<int>(backgrounds_.size()))
    {
        return Brush();
    }

    return backgrounds_[column];
}

void TreeWidgetItem::setBackground(int column, const Brush &brush, bool notify)
{
    if (column < 0)
    {
        return;
    }

    if (column >= static_cast<int>(backgrounds_.size()))
    {
        backgrounds_.resize(static_cast<size_t>(column) + 1);
    }

    if (backgrounds_[column] == brush)
    {
        return;
    }

    backgrounds_[column] = brush;

    if (tree_)
    {
        tree_->itemDataChanged(this, column, notify);
    }
}

TreeWidgetItem *TreeWidgetItem::addChild(const TreeWidgetItem &item)
{
    auto copy = std::make_unique<TreeWidgetItem>(item);
    copy->attach(tree_, this);

    TreeWidgetItem *result = copy.get();
    children_.push_back(std::move(copy));

    if (tree_)
    {
        tree_->inserted(result);
    }

    return result;
}

int TreeWidgetItem::childCount() const
{
    return static_cast<int>(children_.size());
}

TreeWidgetItem *TreeWidgetItem::child(int index) const
{
    if (index < 0 || index >= childCount())
    {
        return nullptr;
    }

    return children_[index].get();
}

void TreeWidgetItem::attach(TreeWidget *tree, TreeWidgetItem *parent)
{
    tree_ = tree;
    parent_ = parent;

    for (auto &child : children_)
    {
        child->attach(tree, this);
    }
}
