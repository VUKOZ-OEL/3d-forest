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

/** @file ExplorerSegmentsWidget.hpp */

#ifndef EXPLORER_SEGMENTS_WIDGET_HPP
#define EXPLORER_SEGMENTS_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <ExplorerWidgetInterface.hpp>
#include <Segments.hpp>
class MainWindow;
class ExplorerSegmentWidget;

// Include Qt.
class QSplitter;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Explorer Segments Widget. */
class ExplorerSegmentsWidget : public ExplorerWidgetInterface
{
    Q_OBJECT

public:
    ExplorerSegmentsWidget(MainWindow *mainWindow,
                           const QIcon &icon,
                           const QString &text);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_SEGMENT;
    }

    virtual bool hasFilter() const { return true; }
    virtual bool isFilterEnabled() const;
    virtual void setFilterEnabled(bool b);

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotAdd();
    void slotDelete();
    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemClicked(QTreeWidgetItem *item, int column);
    void slotItemChanged(QTreeWidgetItem *item, int column);

protected:
    /** Explorer Segments Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    QTreeWidget *tree_;
    QToolButton *addButton_;
    QToolButton *deleteButton_;
    QToolButton *showButton_;
    QToolButton *hideButton_;
    QToolButton *selectAllButton_;
    QToolButton *selectInvertButton_;
    QToolButton *selectNoneButton_;

    QSplitter *splitter_;

    ExplorerSegmentWidget *segmentWidget_;

    Segments segments_;
    QueryFilterSet filter_;
    bool updatesEnabled_;

    void setSegments(const Segments &segments, const QueryFilterSet &filter);
    void dataChanged();
    void filterChanged();

    size_t identifier(const QTreeWidgetItem *item);
    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addTreeItem(size_t index);
};

#endif /* EXPLORER_SEGMENTS_WIDGET_HPP */
