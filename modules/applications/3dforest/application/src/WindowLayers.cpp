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

/** @file WindowLayers.cpp */

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
#include <WindowLayers.hpp>
#include <WindowMain.hpp>
#include <QToolBar>

WindowLayers::WindowLayers(QWidget *parent) : QWidget(parent)
{
    // Table
    tree_ = new QTreeWidget();

    enabledCheckBox_ = new QCheckBox(tr("Enabled"));
    enabledCheckBox_->setToolTip(tr("Enable or disable layer filter"));
    connect(enabledCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(setEnabled(int)));

    invertButton_ = new QPushButton(tr("Invert"));
    invertButton_->setToolTip(tr("Invert the selection"));
    connect(invertButton_, SIGNAL(clicked()), this, SLOT(invertSelection()));

    deselectButton_ = new QPushButton(tr("Deselect"));
    deselectButton_->setToolTip(tr("Dismiss the selection"));
    connect(deselectButton_, SIGNAL(clicked()), this, SLOT(clearSelection()));

    // Menu
    addButton_ = WindowMain::createToolButton(tr("Add"),
                                              tr("Add new layer"),
                                              "file-add");

    editButton_ =
        WindowMain::createToolButton(tr("Edit"), tr("Edit layer"), "file-edit");

    deleteButton_ = WindowMain::createToolButton(tr("Remove"),
                                                 tr("Remove layer"),
                                                 "file-delete");

    connect(addButton_, SIGNAL(clicked()), this, SLOT(toolAdd()));
    connect(editButton_, SIGNAL(clicked()), this, SLOT(toolEdit()));
    connect(deleteButton_, SIGNAL(clicked()), this, SLOT(toolDelete()));

    addButton_->setEnabled(false);
    editButton_->setEnabled(false);
    deleteButton_->setEnabled(false);

    // Tool bar
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(addButton_);
    toolBar->addWidget(editButton_);
    toolBar->addWidget(deleteButton_);
    toolBar->setIconSize(QSize(25, 25));

    // Layout
    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addWidget(enabledCheckBox_);
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

void WindowLayers::toolAdd()
{
}

void WindowLayers::toolEdit()
{
}

void WindowLayers::toolDelete()
{
}

void WindowLayers::setEnabled(int state)
{
    bool checked = (state == Qt::Checked);
    layers_.setEnabled(checked);
    setEnabled(checked);
    emit selectionChanged();
}

void WindowLayers::setEnabled(bool checked)
{
    tree_->setEnabled(checked);
    invertButton_->setEnabled(checked);
    deselectButton_->setEnabled(checked);
}

void WindowLayers::invertSelection()
{
    layers_.setInvertAll();
    updateTree();
    emit selectionChanged();
}

void WindowLayers::clearSelection()
{
    layers_.setEnabledAll(false);
    updateTree();
    emit selectionChanged();
}

void WindowLayers::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        layers_.setEnabled(id, checked);
        emit selectionChanged();
    }
}

void WindowLayers::updateTree()
{
    block();

    size_t i = 0;
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        if (layers_.isEnabled(i))
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

void WindowLayers::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void WindowLayers::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
}

void WindowLayers::addItem(size_t i)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (layers_.isEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(layers_.id(i)));

    item->setText(COLUMN_LABEL, QString::fromStdString(layers_.label(i)));

    // Color legend
    const Vector3<float> &rgb = layers_.color(i);

    QColor color;
    color.setRedF(rgb[0]);
    color.setGreenF(rgb[1]);
    color.setBlueF(rgb[2]);

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}

void WindowLayers::setLayers(const EditorLayers &layers)
{
    block();

    layers_ = layers;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Select") << tr("Id") << tr("Label");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < layers_.size(); i++)
    {
        addItem(i);
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    setEnabled(layers_.isEnabled());
    enabledCheckBox_->setChecked(layers_.isEnabled());

    unblock();
}
