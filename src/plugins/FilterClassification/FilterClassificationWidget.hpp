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

/** @file FilterClassificationWidget.hpp */

#ifndef FILTER_CLASSIFICATION_WIDGET_HPP
#define FILTER_CLASSIFICATION_WIDGET_HPP

// Include 3D Forest.
#include <Classifications.hpp>
#include <Editor.hpp>
class MainWindow;

// Include Qt.
#include <QWidget>
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Filter Classification Widget. */
class FilterClassificationWidget : public QWidget
{
    Q_OBJECT

public:
    FilterClassificationWidget(MainWindow *mainWindow);

    void setFilterEnabled(bool b);

    QSize sizeHint() const override { return QSize(300, 200); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem *item, int column);

protected:
    /** Filter Classification Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    MainWindow *mainWindow_;

    QTreeWidget *tree_;
    QToolButton *showButton_;
    QToolButton *hideButton_;
    QToolButton *selectAllButton_;
    QToolButton *selectInvertButton_;
    QToolButton *selectNoneButton_;
    bool updatesEnabled_;

    Classifications classifications_;
    QueryFilterSet filter_;

    void setClassifications(const Classifications &classifications,
                            const QueryFilterSet &filter);
    void dataChanged();
    void filterChanged();

    size_t identifier(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addTreeItem(size_t index);
};

#endif /* FILTER_CLASSIFICATION_WIDGET_HPP */
