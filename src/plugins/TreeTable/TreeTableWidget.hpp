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

/** @file TreeTableWidget.hpp */

#ifndef TREE_TABLE_WIDGET_HPP
#define TREE_TABLE_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <Segments.hpp>
class MainWindow;

// Include Qt.
#include <QWidget>
class QCheckBox;
class QPushButton;
class QTableWidget;

/** Tree Table Widget. */
class TreeTableWidget : public QWidget
{
    Q_OBJECT

public:
    TreeTableWidget(MainWindow *mainWindow);

    QSize sizeHint() const override { return QSize(700, 200); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

protected slots:
    void showOnlyVisibleTrees();
    void showAllTrees();
    void slotExport();
    void slotCustomContextMenuRequested(const QPoint &pos);

private:
    /** Tree Table Column. */
    enum Column
    {
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_MANAGEMENT_STATUS,
        COLUMN_SPECIES,
        COLUMN_X,
        COLUMN_Y,
        COLUMN_Z,
        COLUMN_HEIGHT,
        COLUMN_DBH,
        COLUMN_AREA,
        COLUMN_STATUS,
        COLUMN_LAST,
    };

    MainWindow *mainWindow_;

    QTableWidget *tableWidget_;
    QPushButton *exportButton_;

    QPushButton *showOnlyVisibleTreesButton_;
    QPushButton *showAllTreesButton_;

    std::unordered_set<size_t> visibleTreesIdList_;

    Segments segments_;
    QueryFilterSet filter_;
    bool updatesEnabled_;

    QString fileName_;

    void setSegments(const Segments &segments, const QueryFilterSet &filter);
    void dataChanged();
    void filterChanged();

    void setTable();
    void setRow(int row, size_t index);
    void setCell(int row, int col, bool value, bool userCheckable);
    void setCell(int row, int col, size_t value);
    void setCell(int row, int col, double value);
    void setCell(int row, int col, const std::string &value);

    void block();
    void unblock();
};

#endif /* TREE_TABLE_WIDGET_HPP */
