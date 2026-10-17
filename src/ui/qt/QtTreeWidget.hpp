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

/** @file QtTreeWidget.hpp */

#ifndef QT_TREE_WIDGET_HPP
#define QT_TREE_WIDGET_HPP

// Include std.
#include <unordered_map>

// Include 3D Forest.
#include <TreeWidget.hpp>
class TreeWidgetItem;
class QtTreeWidgetItem;

// Include Qt.
#include <QTreeWidget>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtTreeWidget. */
class EXPORT_UI_QT QtTreeWidget : public QTreeWidget
{
public:
    explicit QtTreeWidget(TreeWidget *tree, QWidget *parent = nullptr);

    ~QtTreeWidget() override;

private:
    void updateSettings();
    void updateOrder();
    void updateSelection();
    void updateItem(TreeWidgetItem *item);
    void insertItem(TreeWidgetItem *item);
    void resizeColumns();

    TreeWidget *tree_;

    std::unordered_map<TreeWidgetItem *, QtTreeWidgetItem *> items_;
};

#include <WarningsEnable.hpp>

#endif /* QT_TREE_WIDGET_HPP */
