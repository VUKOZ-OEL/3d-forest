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

/** @file FilterManagementStatusWidget.hpp */

#ifndef FILTER_MANAGEMENT_STATUS_WIDGET_HPP
#define FILTER_MANAGEMENT_STATUS_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <ManagementStatusList.hpp>
#include <Widget.hpp>
class Application;
class FilterManagementStatusTreeWidget;
class Splitter;
class ToolButton;
class TreeWidget;
class TreeWidgetItem;

/** Filter Management Status Widget. */
class FilterManagementStatusWidget : public Widget
{
public:
    FilterManagementStatusWidget(Application *app);

    void setFilterEnabled(bool b);

    Size sizeHint() const override { return Size(300, 200); }

    void slotUpdate(const Message &msg);

    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(TreeWidgetItem *item, int column);

protected:
    /** Filter ManagementStatus Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    Application *app_;

    TreeWidget *tree_;
    ToolButton *showButton_;
    ToolButton *hideButton_;
    ToolButton *selectAllButton_;
    ToolButton *selectInvertButton_;
    ToolButton *selectNoneButton_;
    bool updatesEnabled_;

    Splitter *splitter_;

    FilterManagementStatusTreeWidget *treeWidget_;

    ManagementStatusList managementStatus_;
    QueryFilterSet filter_;

    // New data.
    void receivedSegments();
    void receivedManagementStatusList();
    void sendFilter();

    size_t identifier(const TreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addTreeItem(size_t index);
};

#endif /* FILTER_MANAGEMENT_STATUS_WIDGET_HPP */
