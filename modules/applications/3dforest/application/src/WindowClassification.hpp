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

/** @file WindowClassification.hpp */

#ifndef WINDOW_CLASSIFICATION_HPP
#define WINDOW_CLASSIFICATION_HPP

#include <EditorClassification.hpp>
#include <QWidget>

class QCheckBox;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Window Classification. */
class WindowClassification : public QWidget
{
    Q_OBJECT

public:
    /** Window Classification Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    WindowClassification(QWidget *parent);

    const EditorClassification &classification() const
    {
        return classification_;
    }
    void setClassification(const EditorClassification &classification);

public slots:
    void setEnabled(int state);
    void setEnabled(bool checked);
    void invertSelection();
    void clearSelection();
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void selectionChanged();

protected:
    EditorClassification classification_;
    QTreeWidget *tree_;
    QCheckBox *enabledCheckBox_;
    QPushButton *invertButton_;
    QPushButton *deselectButton_;

    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);
};

#endif /* WINDOW_CLASSIFICATION_HPP */
