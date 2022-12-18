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

/** @file ProjectNavigatorItemLayers.hpp */

#ifndef PROJECT_NAVIGATOR_ITEM_LAYERS_HPP
#define PROJECT_NAVIGATOR_ITEM_LAYERS_HPP

#include <Editor.hpp>
#include <Layers.hpp>
#include <ProjectNavigatorItem.hpp>

class MainWindow;

class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Project Navigator Layers. */
class ProjectNavigatorItemLayers : public ProjectNavigatorItem
{
    Q_OBJECT

public:
    ProjectNavigatorItemLayers(MainWindow *mainWindow,
                               const QIcon &icon,
                               const QString &text);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_LAYER;
    }

    virtual bool hasFilter() const { return false; }

public slots:
    void slotUpdate(const QSet<Editor::Type> &target);

    void slotAdd();
    void slotDelete();
    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem *item, int column);

protected:
    /** Project Navigator Layers Column. */
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
    Layers layers_;

    void dataChanged();
    void filterChanged();

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);

    void setLayers(const Layers &layers);
};

#endif /* PROJECT_NAVIGATOR_ITEM_LAYERS_HPP */
