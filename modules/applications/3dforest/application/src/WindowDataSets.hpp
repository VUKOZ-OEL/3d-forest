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

#include <Editor.hpp>
#include <QWidget>

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
        COLUMN_ID,
        COLUMN_CHECKED,
        COLUMN_FILE_NAME,
        COLUMN_DATE_CREATED,
        COLUMN_LAST,
    };

    explicit WindowDataSets(QWidget *parent = nullptr);
    ~WindowDataSets();

    void updateEditor(const Editor &editor);

public slots:
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void itemChangedCheckState(size_t id, bool checked);

protected:
    QTreeWidget *dataSets_;
};

#endif /* WINDOW_DATA_SETS_HPP */
