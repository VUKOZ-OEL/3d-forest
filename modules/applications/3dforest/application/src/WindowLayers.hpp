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

/** @file WindowLayers.hpp */

#ifndef WINDOW_LAYERS_HPP
#define WINDOW_LAYERS_HPP

#include <EditorLayers.hpp>
#include <QDialog>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class WindowMain;

/** Window Layers. */
class WindowLayers : public QWidget
{
    Q_OBJECT

public:
    /** Window Layers Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    WindowLayers(WindowMain *parent);

    const EditorLayers &layers() const { return layers_; }
    void setLayers(const EditorLayers &layers);

public slots:
    void toolAdd();
    void toolEdit();
    void toolDelete();

    void setEnabled(int state);
    void setEnabled(bool checked);
    void invertSelection();
    void clearSelection();

    void itemSelectionChanged();
    void itemChanged(QTreeWidgetItem *item, int column);

signals:
    void dataChanged();
    void selectionChanged();

protected:
    WindowMain *windowMain_;

    QTreeWidget *tree_;
    QCheckBox *enabledCheckBox_;
    QPushButton *invertButton_;
    QPushButton *deselectButton_;
    QToolButton *addButton_;
    QToolButton *editButton_;
    QToolButton *deleteButton_;

    EditorLayers layers_;

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);
};

/** Window Layers Edit. */
class WindowLayersEdit : public QDialog
{
    Q_OBJECT

public:
    QLineEdit *labelEdit_;
    QColor color_;

    WindowLayersEdit(QWidget *parent,
                     const QString &windowTitle,
                     const QString &buttonText,
                     const QString &label,
                     const QColor &color);

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

#endif /* WINDOW_LAYERS_HPP */
