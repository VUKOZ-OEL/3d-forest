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

/** @file AlgorithmTabWidget.hpp */

#ifndef ALGORITHM_TAB_WIDGET_HPP
#define ALGORITHM_TAB_WIDGET_HPP

#include <vector>

#include <AlgorithmWidgetInterface.hpp>
#include <Editor.hpp>
class MainWindow;

#include <QWidget>
class QVBoxLayout;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

/** Algorithm Tab Widget. */
class AlgorithmTabWidget : public QWidget
{
    Q_OBJECT

public:
    AlgorithmTabWidget(MainWindow *mainWindow);
    virtual ~AlgorithmTabWidget();

    void addItem(AlgorithmWidgetInterface *widget);

public slots:
    void slotItemClicked(QTreeWidgetItem *item, int column);

private:
    /** Algorithm Tab Column. */
    enum Column
    {
        COLUMN_LABEL,
        COLUMN_ID,
        COLUMN_LAST
    };

    MainWindow *mainWindow_;

    QTreeWidget *treeWidget_;
    QVBoxLayout *widgetsLayout_;

    std::vector<AlgorithmWidgetInterface *> widgets_;

    size_t index(const QTreeWidgetItem *item);
    void block();
    void unblock();
    void setTabVisible(size_t index);
};

#endif /* ALGORITHM_TAB_WIDGET_HPP */
