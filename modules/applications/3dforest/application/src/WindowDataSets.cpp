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

/** @file WindowDataSets.cpp */

#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowDataSets.hpp>
#include <WindowMain.hpp>
#include <QToolBar>

WindowDataSets::WindowDataSets(QWidget *parent) : QWidget(parent)
{
    // Table
    tree_ = new QTreeWidget();

    invertButton_ = new QPushButton(tr("Invert"));
    invertButton_->setToolTip(tr("Invert the selection"));
    connect(invertButton_, SIGNAL(clicked()), this, SLOT(invertSelection()));

    deselectButton_ = new QPushButton(tr("Deselect"));
    deselectButton_->setToolTip(tr("Dismiss the selection"));
    connect(deselectButton_, SIGNAL(clicked()), this, SLOT(clearSelection()));

    // Menu
    addButton_ = WindowMain::createToolButton(tr("Add"),
                                              tr("Add new data set"),
                                              "file-add");

    editButton_ = WindowMain::createToolButton(tr("Edit"),
                                               tr("Edit data set"),
                                               "file-edit");

    deleteButton_ = WindowMain::createToolButton(tr("Remove"),
                                                 tr("Remove data set"),
                                                 "file-delete");

    connect(addButton_, SIGNAL(clicked()), this, SLOT(toolAdd()));
    connect(editButton_, SIGNAL(clicked()), this, SLOT(toolEdit()));
    connect(deleteButton_, SIGNAL(clicked()), this, SLOT(toolDelete()));

    // Tool bar
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(addButton_);
    toolBar->addWidget(editButton_);
    toolBar->addWidget(deleteButton_);
    toolBar->setIconSize(QSize(25, 25));

    // Layout
    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(invertButton_);
    controlLayout->addWidget(deselectButton_);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);
    mainLayout->addLayout(controlLayout);
    setLayout(mainLayout);
}

void WindowDataSets::toolAdd()
{
}

void WindowDataSets::toolEdit()
{
}

void WindowDataSets::toolDelete()
{
}

void WindowDataSets::invertSelection()
{
    dataSets_.setInvertAll();
    updateTree();
    emit selectionChanged();
}

void WindowDataSets::clearSelection()
{
    dataSets_.setEnabledAll(false);
    updateTree();
    emit selectionChanged();
}

void WindowDataSets::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        dataSets_.setEnabled(id, checked);
        emit selectionChanged();
    }
}

void WindowDataSets::updateTree()
{
    block();

    size_t i = 0;
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        if (dataSets_.isEnabled(i))
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        else
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }

        i++;
        ++it;
    }

    unblock();
}

void WindowDataSets::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void WindowDataSets::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
}

void WindowDataSets::addItem(size_t i)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (dataSets_.isEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(dataSets_.id(i)));

    // item->setText(COLUMN_LABEL, QString::fromStdString(dataSets_.label(i)));

    item->setText(COLUMN_FILE_NAME,
                  QString::fromStdString(dataSets_.fileName(i)));
    item->setText(COLUMN_DATE_CREATED,
                  QString::fromStdString(dataSets_.dateCreated(i)));

    // Color legend
    const Vector3<float> &rgb = dataSets_.color(i);

    QColor color;
    color.setRedF(rgb[0]);
    color.setGreenF(rgb[1]);
    color.setBlueF(rgb[2]);

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}

void WindowDataSets::setDataSets(const EditorDataSets &dataSets)
{
    block();

    dataSets_ = dataSets;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Select") << tr("Id") << tr("File name") << tr("Date");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < dataSets_.size(); i++)
    {
        addItem(i);
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    // Sort Content
    tree_->setSortingEnabled(true);
    tree_->sortItems(COLUMN_ID, Qt::AscendingOrder);

    unblock();
}
