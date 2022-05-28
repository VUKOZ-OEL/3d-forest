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

/** @file ProjectNavigatorLayers.hpp */

#ifndef PROJECT_NAVIGATOR_LAYERS_HPP
#define PROJECT_NAVIGATOR_LAYERS_HPP

#include <LayerList.hpp>

#include <QWidget>

class MainWindow;

class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Project Navigator Layers. */
class ProjectNavigatorLayers : public QWidget
{
    Q_OBJECT

public:
    /** Project Navigator Layers Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    ProjectNavigatorLayers(MainWindow *mainWindow);

public slots:
    void slotAdd();
    void slotDelete();
    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem *item, int column);

    void slotUpdate();

protected:
    MainWindow *mainWindow_;
    QTreeWidget *tree_;
    QToolButton *addButton_;
    QToolButton *deleteButton_;
    QToolButton *showButton_;
    QToolButton *hideButton_;
    QToolButton *selectAllButton_;
    QToolButton *selectInvertButton_;
    QToolButton *selectNoneButton_;
    LayerList layers_;

    void dataChanged();
    void filterChanged();

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);

    void setLayers(const LayerList &layers);
};

#endif /* PROJECT_NAVIGATOR_LAYERS_HPP */
