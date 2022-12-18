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

/** @file ProjectNavigatorTree.hpp */

#ifndef PROJECT_NAVIGATOR_TREE_HPP
#define PROJECT_NAVIGATOR_TREE_HPP

#include <vector>

#include <Editor.hpp>
#include <ProjectNavigatorItem.hpp>
#include <Settings.hpp>

#include <QWidget>

class MainWindow;

class QVBoxLayout;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

/** Project Navigator Tree. */
class ProjectNavigatorTree : public QWidget
{
    Q_OBJECT

public:
    ProjectNavigatorTree(MainWindow *mainWindow);

    void addItem(ProjectNavigatorItem *widget,
                 const QIcon &icon,
                 const QString &label);

public slots:
    void slotUpdate(const QSet<Editor::Type> &target);
    void slotItemChanged(QTreeWidgetItem *item, int column);
    void slotItemClicked(QTreeWidgetItem *item, int column);

private:
    /** Project Navigator Tree Column. */
    enum Column
    {
        COLUMN_COLOR,
        COLUMN_FILTER,
        COLUMN_LABEL,
        COLUMN_ID,
        COLUMN_LAST
    };

    MainWindow *mainWindow_;
    QTreeWidget *treeWidget_;
    QLabel *icon_;
    QLabel *label_;
    QVBoxLayout *tabLayout_;
    std::vector<ProjectNavigatorItem *> tabList_;
    SettingsView settings_;

    size_t index(const QTreeWidgetItem *item);
    void block();
    void unblock();

    void setTabVisible(size_t index, QTreeWidgetItem *item);

    void applySettingsIn(const SettingsView &settings);
    void applySettingsOut();
};

#endif /* PROJECT_NAVIGATOR_TREE_HPP */
