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
#include <QColorDialog>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <WindowLayers.hpp>
#include <WindowMain.hpp>

WindowLayers::WindowLayers(WindowMain *parent)
    : QWidget(parent),
      windowMain_(parent)
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
    invertButton_->setToolTip(tr("Inverts visibility"));
    connect(invertButton_, SIGNAL(clicked()), this, SLOT(invertSelection()));

    deselectButton_ = new QPushButton(tr("Hide all"));
    deselectButton_->setToolTip(tr("Hides all layers"));
    connect(deselectButton_, SIGNAL(clicked()), this, SLOT(clearSelection()));

    // Menu
    addButton_ = WindowMain::createToolButton(tr("Add"),
                                              tr("Adds new layer"),
                                              "file-add");

    editButton_ = WindowMain::createToolButton(tr("Edit"),
                                               tr("Edits selected layer"),
                                               "file-edit");

    deleteButton_ = WindowMain::createToolButton(tr("Remove"),
                                                 tr("Removes selected layer"),
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
    // Dialog
    WindowLayersNew dialog(windowMain_);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    // Apply
    float r = static_cast<float>(dialog.color_.redF());
    float g = static_cast<float>(dialog.color_.greenF());
    float b = static_cast<float>(dialog.color_.blueF());
    Vector3<float> color(r, g, b);

    EditorLayer newLayer;
    newLayer.set(layers_.unusedId(),
                 dialog.labelEdit_->text().toStdString(),
                 true,
                 color);

    layers_.push_back(newLayer);
    setLayers(layers_);

    // Update
    emit selectionChanged();
}

void WindowLayers::toolEdit()
{
}

void WindowLayers::toolDelete()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() < 1)
    {
        return;
    }

    QTreeWidgetItem *item = items.at(0);
    size_t idx = index(item);
    if (idx > 0)
    {
        layers_.erase(idx);
        delete item;
        emit selectionChanged();
    }
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

void WindowLayers::itemSelectionChanged()
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

void WindowLayers::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        layers_.setEnabled(index(item), checked);
        emit selectionChanged();
    }
}

size_t WindowLayers::index(const QTreeWidgetItem *item)
{
    return layers_.index(item->text(COLUMN_ID).toULong());
}

void WindowLayers::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t idx = index(*it);

        if (layers_.isEnabled(idx))
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

void WindowLayers::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void WindowLayers::unblock()
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
    labels << tr("Visible") << tr("Id") << tr("Label");
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

WindowLayersNew::WindowLayersNew(QWidget *parent)
    : QDialog(parent),
      color_(255, 255, 255)
{
    // Widgets
    acceptButton_ = new QPushButton(tr("Create"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(setResultAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(setResultReject()));

    labelEdit_ = new QLineEdit("label");

    colorButton_ = new QPushButton(tr("Custom"));
    updateColor();
    connect(colorButton_, SIGNAL(clicked()), this, SLOT(setColor()));

    // Layout
    QGridLayout *gridLayout = new QGridLayout;
    int row = 0;
    gridLayout->addWidget(new QLabel(tr("Label")), row, 0);
    gridLayout->addWidget(labelEdit_, row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Color")), row, 0);
    gridLayout->addWidget(colorButton_, row, 1);
    row++;

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(gridLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window
    setWindowTitle("New Layer");
    setMaximumWidth(width());
    setMaximumHeight(height());
}

void WindowLayersNew::setResultAccept()
{
    close();
    setResult(QDialog::Accepted);
}

void WindowLayersNew::setResultReject()
{
    close();
    setResult(QDialog::Rejected);
}

void WindowLayersNew::setColor()
{
    QColorDialog dialog(color_, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    color_ = dialog.selectedColor();
    updateColor();
}

void WindowLayersNew::updateColor()
{
    QPixmap pixmap(25, 25);
    pixmap.fill(color_);

    QIcon icon(pixmap);

    colorButton_->setIcon(icon);
    colorButton_->setIconSize(QSize(10, 10));
}
