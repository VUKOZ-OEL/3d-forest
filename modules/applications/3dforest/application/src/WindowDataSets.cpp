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

#include <Editor.hpp>
#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowDataSets.hpp>
#include <WindowFileImport.hpp>
#include <WindowMain.hpp>

WindowDataSets::WindowDataSets(WindowMain *parent, Editor *editor)
    : QWidget(parent),
      windowMain_(parent),
      editor_(editor)
{
    // Table
    tree_ = new QTreeWidget();

    invertButton_ = new QPushButton(tr("Invert"));
    invertButton_->setToolTip(tr("Invert visibility"));
    connect(invertButton_, SIGNAL(clicked()), this, SLOT(invertSelection()));

    deselectButton_ = new QPushButton(tr("Hide all"));
    deselectButton_->setToolTip(tr("Hide all data sets"));
    connect(deselectButton_, SIGNAL(clicked()), this, SLOT(clearSelection()));

    // Menu
    addButton_ = WindowMain::createToolButton(tr("Add"),
                                              tr("Add new data set"),
                                              "file-add");

    editButton_ = WindowMain::createToolButton(tr("Edit"),
                                               tr("Edit selected data set"),
                                               "file-edit");

    deleteButton_ = WindowMain::createToolButton(tr("Remove"),
                                                 tr("Remove selected data set"),
                                                 "file-delete");

    connect(addButton_, SIGNAL(clicked()), this, SLOT(toolAdd()));
    connect(editButton_, SIGNAL(clicked()), this, SLOT(toolEdit()));
    connect(deleteButton_, SIGNAL(clicked()), this, SLOT(toolDelete()));

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
    WindowFileImport::import(windowMain_, editor_);
}

void WindowDataSets::toolEdit()
{
    // Item
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() < 1)
    {
        return;
    }

    QTreeWidgetItem *item = items.at(0);
    size_t idx = index(item);

    // Dialog
    WindowDataSetsEdit dialog(windowMain_);

    Vector3<double> t = dataSets_.translation(idx);
    for (size_t i = 0; i < 3; i++)
    {
        dialog.offsetSpinBox_[i]->setMinimum(-1e8);
        dialog.offsetSpinBox_[i]->setMaximum(1e8);
        dialog.offsetSpinBox_[i]->setValue(t[i]);
    }

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    // Apply
    for (size_t i = 0; i < 3; i++)
    {
        t[i] = dialog.offsetSpinBox_[i]->value();
    }
    dataSets_.setTranslation(idx, t);

    // Update
    emit selectionChanged();
}

void WindowDataSets::toolDelete()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() < 1)
    {
        return;
    }

    QTreeWidgetItem *item = items.at(0);
    size_t idx = index(item);
    dataSets_.remove(idx);
    delete item;
    emit selectionChanged();
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

void WindowDataSets::itemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        editButton_->setEnabled(true);
        deleteButton_->setEnabled(true);
    }
    else
    {
        editButton_->setEnabled(false);
        deleteButton_->setEnabled(false);
    }
}

void WindowDataSets::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        dataSets_.setEnabled(index(item), checked);
        emit selectionChanged();
    }
}

size_t WindowDataSets::index(const QTreeWidgetItem *item)
{
    return dataSets_.index(item->text(COLUMN_ID).toULong());
}

void WindowDataSets::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t idx = index(*it);

        if (dataSets_.isEnabled(idx))
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        else
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }

        ++it;
    }

    unblock();
}

void WindowDataSets::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void WindowDataSets::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(itemChanged(QTreeWidgetItem *, int)));
    connect(tree_,
            SIGNAL(itemSelectionChanged()),
            this,
            SLOT(itemSelectionChanged()));
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

    item->setText(COLUMN_LABEL, QString::fromStdString(dataSets_.label(i)));
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
    labels << tr("Visible") << tr("Id") << tr("Label") << tr("Date");
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

WindowDataSetsEdit::WindowDataSetsEdit(QWidget *parent) : QDialog(parent)
{
    // Widgets
    acceptButton_ = new QPushButton(tr("Apply"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(setResultAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(setResultReject()));

    QGroupBox *offsetGroup = new QGroupBox(tr("Offset"));

    for (int i = 0; i < 3; i++)
    {
        offsetSpinBox_[i] = new QDoubleSpinBox;
        offsetSpinBox_[i]->setDecimals(6);
    }

    // Layout
    QGridLayout *offsetLayout = new QGridLayout;
    int row = 0;
    offsetLayout->addWidget(new QLabel(tr("x")), row, 0);
    offsetLayout->addWidget(offsetSpinBox_[0], row, 1);
    row++;
    offsetLayout->addWidget(new QLabel(tr("y")), row, 0);
    offsetLayout->addWidget(offsetSpinBox_[1], row, 1);
    row++;
    offsetLayout->addWidget(new QLabel(tr("z")), row, 0);
    offsetLayout->addWidget(offsetSpinBox_[2], row, 1);
    row++;
    offsetGroup->setLayout(offsetLayout);

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(offsetGroup);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window
    setWindowTitle("Edit Data Set");
    setMaximumWidth(width());
    setMaximumHeight(height());
}

void WindowDataSetsEdit::setResultAccept()
{
    close();
    setResult(QDialog::Accepted);
}

void WindowDataSetsEdit::setResultReject()
{
    close();
    setResult(QDialog::Rejected);
}
