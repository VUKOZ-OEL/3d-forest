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

/** @file WindowDataSets.hpp */

#ifndef WINDOW_DATA_SETS_HPP
#define WINDOW_DATA_SETS_HPP

#include <EditorDataSets.hpp>
#include <QWidget>

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Window Data Sets. */
class WindowDataSets : public QWidget
{
    Q_OBJECT

public:
    /** Window Data Sets Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_FILE_NAME,
        COLUMN_DATE_CREATED,
        COLUMN_LAST,
    };

    WindowDataSets(QWidget *parent);

    const EditorDataSets &dataSets() const { return dataSets_; }
    void setDataSets(const EditorDataSets &dataSets);

public slots:
    void invertSelection();
    void clearSelection();
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void selectionChanged();

protected:
    EditorDataSets dataSets_;
    QTreeWidget *tree_;
    QPushButton *invertButton_;
    QPushButton *deselectButton_;

    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);
};

#endif /* WINDOW_DATA_SETS_HPP */
