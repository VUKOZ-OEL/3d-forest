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

#include <ColorPalette.hpp>
#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowClassification.hpp>

WindowClassification::WindowClassification(QWidget *parent) : QWidget(parent)
{
    // Table
    tree_ = new QTreeWidget();

    enabledCheckBox_ = new QCheckBox(tr("Enabled"));
    enabledCheckBox_->setToolTip(tr("Enable or disable classification filter"));
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

    // Layout
    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addWidget(enabledCheckBox_);
    controlLayout->addStretch();
    controlLayout->addWidget(invertButton_);
    controlLayout->addWidget(deselectButton_);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(tree_);
    mainLayout->addLayout(controlLayout);
    setLayout(mainLayout);
}

void WindowClassification::setEnabled(int state)
{
    bool checked = (state == Qt::Checked);
    classification_.setEnabled(checked);
    setEnabled(checked);
    emit selectionChanged();
}

void WindowClassification::setEnabled(bool checked)
{
    tree_->setEnabled(checked);
    invertButton_->setEnabled(checked);
    deselectButton_->setEnabled(checked);
}

void WindowClassification::invertSelection()
{
    classification_.setInvertAll();
    updateTree();
    emit selectionChanged();
}

void WindowClassification::clearSelection()
{
    classification_.setEnabledAll(false);
    updateTree();
    emit selectionChanged();
}

void WindowClassification::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        classification_.setEnabled(id, checked);
        emit selectionChanged();
    }
}

void WindowClassification::updateTree()
{
    block();

    size_t i = 0;
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        if (classification_.isEnabled(i))
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

void WindowClassification::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void WindowClassification::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
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

    // Color legend
    if (i < ColorPalette::Classification.size())
    {
        const Vector3<float> &rgb = ColorPalette::Classification[i];

        QColor color;
        color.setRedF(rgb[0]);
        color.setGreenF(rgb[1]);
        color.setBlueF(rgb[2]);

        QBrush brush(color, Qt::SolidPattern);
        item->setBackground(COLUMN_ID, brush);
        // brush.setColor(QColor(0, 0, 0));
        // item->setForeground(COLUMN_ID, brush);
    }
}

void WindowClassification::setClassification(
    const EditorClassification &classification)
{
    block();

    classification_ = classification;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Select") << tr("Class") << tr("Label");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < classification_.size(); i++)
    {
        addItem(i);
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    setEnabled(classification_.isEnabled());
    enabledCheckBox_->setChecked(classification_.isEnabled());

    unblock();
}
