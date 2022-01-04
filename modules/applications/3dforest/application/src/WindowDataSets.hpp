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

#include <EditorDatasets.hpp>
#include <QDialog>
#include <QWidget>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class WindowMain;
class Editor;

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
        COLUMN_LABEL,
        COLUMN_DATE_CREATED,
        COLUMN_LAST,
    };

    WindowDataSets(WindowMain *parent, Editor *editor);

    const EditorDatasets &datasets() const { return datasets_; }
    void setDatasets(const EditorDatasets &datasets);

public slots:
    void toolAdd();
    void toolEdit();
    void toolDelete();

    void invertSelection();
    void clearSelection();
    void itemSelectionChanged();
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void dataChanged();
    void selectionChanged();

protected:
    WindowMain *windowMain_;
    Editor *editor_;

    QTreeWidget *tree_;
    QPushButton *invertButton_;
    QPushButton *deselectButton_;
    QToolButton *addButton_;
    QToolButton *editButton_;
    QToolButton *deleteButton_;

    EditorDatasets datasets_;

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);
};

/** Window Data Sets Edit. */
class WindowDataSetsEdit : public QDialog
{
    Q_OBJECT

public:
    QColor color_;
    QLineEdit *labelEdit_;
    QDoubleSpinBox *offsetSpinBox_[3];

    WindowDataSetsEdit(QWidget *parent,
                       const QString &windowTitle,
                       const QString &buttonText,
                       const QString &label,
                       const QColor &color,
                       const Vector3<double> &offset);

public slots:
    void setResultAccept();
    void setResultReject();
    void setColor();

protected:
    QPushButton *acceptButton_;
    QPushButton *rejectButton_;

    QPushButton *colorButton_;

    void updateColor();
};

#endif /* WINDOW_DATA_SETS_HPP */
