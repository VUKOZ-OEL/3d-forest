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
#include <QColorDialog>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QGroupBox>
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

    QString label = QString::fromStdString(datasets_.label(idx));
    Vector3<float> rgb = datasets_.color(idx);
    Vector3<double> offset = datasets_.translation(idx);
    Vector3<double> scale = datasets_.scalingFile(idx);

    QColor color;
    color.setRgbF(rgb[0], rgb[1], rgb[2]);

    // Dialog
    WindowDataSetsEdit dialog(windowMain_,
                              "Edit Data Set",
                              "Apply",
                              label,
                              color,
                              offset,
                              scale);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    // Apply
    label = dialog.labelEdit_->text();

    float r = static_cast<float>(dialog.color_.redF());
    float g = static_cast<float>(dialog.color_.greenF());
    float b = static_cast<float>(dialog.color_.blueF());
    rgb.set(r, g, b);

    for (size_t i = 0; i < 3; i++)
    {
        offset[i] = dialog.offsetSpinBox_[i]->value();
    }

    datasets_.setLabel(idx, label.toStdString());
    datasets_.setColor(idx, rgb);
    datasets_.setTranslation(idx, offset);

    setDatasets(datasets_);

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
    datasets_.erase(idx);
    delete item;

    emit dataChanged();
}

void WindowDataSets::invertSelection()
{
    datasets_.setInvertAll();
    updateTree();
    emit selectionChanged();
}

void WindowDataSets::clearSelection()
{
    datasets_.setEnabledAll(false);
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

        datasets_.setEnabled(index(item), checked);
        emit selectionChanged();
    }
}

size_t WindowDataSets::index(const QTreeWidgetItem *item)
{
    return datasets_.index(item->text(COLUMN_ID).toULong());
}

void WindowDataSets::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t idx = index(*it);

        if (datasets_.isEnabled(idx))
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

    if (datasets_.isEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(datasets_.id(i)));

    item->setText(COLUMN_LABEL, QString::fromStdString(datasets_.label(i)));
    item->setText(COLUMN_DATE_CREATED,
                  QString::fromStdString(datasets_.dateCreated(i)));

    // Color legend
    const Vector3<float> &rgb = datasets_.color(i);

    QColor color;
    color.setRedF(rgb[0]);
    color.setGreenF(rgb[1]);
    color.setBlueF(rgb[2]);

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}

void WindowDataSets::setDatasets(const EditorDatasets &datasets)
{
    block();

    datasets_ = datasets;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Label") << tr("Date");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < datasets_.size(); i++)
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

WindowDataSetsEdit::WindowDataSetsEdit(QWidget *parent,
                                       const QString &windowTitle,
                                       const QString &buttonText,
                                       const QString &label,
                                       const QColor &color,
                                       const Vector3<double> &offset,
                                       const Vector3<double> &scale)
    : QDialog(parent),
      color_(color)
{
    // Widgets
    acceptButton_ = new QPushButton(buttonText);
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(setResultAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(setResultReject()));

    labelEdit_ = new QLineEdit(label);

    colorButton_ = new QPushButton(tr("Custom"));
    updateColor();
    connect(colorButton_, SIGNAL(clicked()), this, SLOT(setColor()));

    for (size_t i = 0; i < 3; i++)
    {
        offsetSpinBox_[i] = new QDoubleSpinBox;
        offsetSpinBox_[i]->setDecimals(6);
        offsetSpinBox_[i]->setMinimum(-1e8);
        offsetSpinBox_[i]->setMaximum(1e8);
        offsetSpinBox_[i]->setValue(offset[i]);

        scaleSpinBox_[i] = new QDoubleSpinBox;
        scaleSpinBox_[i]->setDecimals(6);
        scaleSpinBox_[i]->setMinimum(-1e8);
        scaleSpinBox_[i]->setMaximum(1e8);
        scaleSpinBox_[i]->setValue(scale[i]);
        scaleSpinBox_[i]->setDisabled(true);
    }

    // Layout
    QGridLayout *gridLayout = new QGridLayout;
    int row = 0;

    gridLayout->addWidget(new QLabel(tr("Label")), row, 0);
    gridLayout->addWidget(labelEdit_, row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Color")), row, 0);
    gridLayout->addWidget(colorButton_, row, 1);
    row++;

    gridLayout->addWidget(new QLabel(tr("Offset x")), row, 0);
    gridLayout->addWidget(offsetSpinBox_[0], row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Offset y")), row, 0);
    gridLayout->addWidget(offsetSpinBox_[1], row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Offset z")), row, 0);
    gridLayout->addWidget(offsetSpinBox_[2], row, 1);
    row++;

    gridLayout->addWidget(new QLabel(tr("Scale x")), row, 0);
    gridLayout->addWidget(scaleSpinBox_[0], row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Scale y")), row, 0);
    gridLayout->addWidget(scaleSpinBox_[1], row, 1);
    row++;
    gridLayout->addWidget(new QLabel(tr("Scale z")), row, 0);
    gridLayout->addWidget(scaleSpinBox_[2], row, 1);
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
    setWindowTitle(windowTitle);
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

void WindowDataSetsEdit::setColor()
{
    QColorDialog dialog(color_, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    color_ = dialog.selectedColor();
    updateColor();
}

void WindowDataSetsEdit::updateColor()
{
    QPixmap pixmap(25, 25);
    pixmap.fill(color_);

    QIcon icon(pixmap);

    colorButton_->setIcon(icon);
    colorButton_->setIconSize(QSize(10, 10));
}
