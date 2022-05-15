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

/** @file GuiProjectDatasets.hpp */

#ifndef GUI_PROJECT_DATASETS_HPP
#define GUI_PROJECT_DATASETS_HPP

#include <EditorDatasets.hpp>

#include <QWidget>

class GuiWindowMain;

class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Gui Project Datasets. */
class GuiProjectDatasets : public QWidget
{
    Q_OBJECT

public:
    /** Gui Project Datasets Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_DATE_CREATED,
        COLUMN_LAST,
    };

    GuiProjectDatasets(GuiWindowMain *mainWindow);

public slots:
    void slotAdd();
    void slotDelete();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem *item, int column);

    void slotUpdate();

protected:
    GuiWindowMain *mainWindow_;
    QTreeWidget *tree_;
    QToolButton *addButton_;
    QToolButton *deleteButton_;
    QToolButton *selectAllButton_;
    QToolButton *selectInvertButton_;
    QToolButton *selectNoneButton_;
    EditorDatasets datasets_;

    void dataChanged();
    void selectionChanged();

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);

    void setDatasets(const EditorDatasets &datasets);
};

#endif /* GUI_PROJECT_DATASETS_HPP */
