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

/** @file WindowClassification.cpp */

#include <QDebug>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowClassification.hpp>

WindowClassification::WindowClassification(QWidget *parent) : QWidget(parent)
{
    // Table
    tree_ = new QTreeWidget();

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(tree_);
    setLayout(mainLayout);
}

void WindowClassification::updateAll(bool checked)
{
    // Qt::CheckState state;
    // if (checked)
    // {
    //     state = Qt::Checked;
    // }
    // else
    // {
    //     state = Qt::Unchecked;
    // }

    QTreeWidgetItemIterator it(tree_);
    ++it;
    while (*it)
    {
        //(*it)->setCheckState(COLUMN_CHECKED, state);
        (*it)->setDisabled(checked);
        ++it;
    }
}

void WindowClassification::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        // #id is now checked or unchecked
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        if (id == classification_.indexAll())
        {
            (void)blockSignals(true);
            updateAll(checked);
            (void)blockSignals(false);
        }

        classification_.setEnabled(id, checked);

        emit selectionChanged();
    }
}

void WindowClassification::addItem(size_t i)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (classification_.isEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(i));

    item->setText(COLUMN_LABEL,
                  QString::fromStdString(classification_.label(i)));
}

void WindowClassification::setClassification(
    const EditorClassification &classification)
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);

    (void)blockSignals(true);

    classification_ = classification;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Select") << tr("Class") << tr("Label");
    tree_->setHeaderLabels(labels);

    // Content
    addItem(classification_.indexAll());
    for (size_t i = 0; i < classification_.size() - 1; i++)
    {
        addItem(i);
    }

    updateAll(classification_.isEnabled(classification_.indexAll()));

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    (void)blockSignals(false);

    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
}
