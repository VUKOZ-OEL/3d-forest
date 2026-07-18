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

/** @file FilterFilesWidget.hpp */

#ifndef FILTER_FILES_WIDGET_HPP
#define FILTER_FILES_WIDGET_HPP

// Include 3D Forest.
#include <Widget.hpp>
#include <Datasets.hpp>
#include <Editor.hpp>
class Application;
class ToolButton;
class TreeWidget;
class TreeWidgetItem;

/** Filter Files Widget. */
class FilterFilesWidget : public Widget
{
public:
    FilterFilesWidget(Application *app);

    void setFilterEnabled(bool b);

    Size sizeHint() const override { return Size(300, 200); }

    void slotUpdate(void *sender, const std::set<Editor::Type> &target);

    void slotAdd();
    void slotDelete();
    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(TreeWidgetItem *item, int column);

protected:
    /** Filter Files Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_DATE_CREATED,
        COLUMN_LAST,
    };

    Application *app_;

    TreeWidget *tree_;
    ToolButton *addButton_;
    ToolButton *deleteButton_;
    ToolButton *showButton_;
    ToolButton *hideButton_;
    ToolButton *selectAllButton_;
    ToolButton *selectInvertButton_;
    ToolButton *selectNoneButton_;

    Datasets datasets_;
    QueryFilterSet filter_;
    bool updatesEnabled_;

    void setDatasets(const Datasets &datasets, const QueryFilterSet &filter);
    void dataChanged();
    void filterChanged();

    size_t identifier(const TreeWidgetItem *item);
    size_t index(const TreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addTreeItem(size_t index);
};

#endif /* FILTER_FILES_WIDGET_HPP */
