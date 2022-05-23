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

/** @file GuiProjectDatasets.cpp */

#include <GuiIconTheme.hpp>
#include <GuiPluginImport.hpp>
#include <GuiProjectDatasets.hpp>
#include <GuiWindowMain.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

#define ICON(name) (GuiIconTheme(":/project/", name))

GuiProjectDatasets::GuiProjectDatasets(GuiWindowMain *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    // Table
    tree_ = new QTreeWidget();

    // Tool bar buttons
    GuiWindowMain::createToolButton(&addButton_,
                                    tr("Add"),
                                    tr("Add new data set"),
                                    ICON("plus"),
                                    this,
                                    SLOT(slotAdd()));

    GuiWindowMain::createToolButton(&deleteButton_,
                                    tr("Remove"),
                                    tr("Remove selected data set"),
                                    ICON("minus"),
                                    this,
                                    SLOT(slotDelete()));
    deleteButton_->setEnabled(false);

    GuiWindowMain::createToolButton(&selectAllButton_,
                                    tr("Select all"),
                                    tr("Select all"),
                                    ICON("select_all"),
                                    this,
                                    SLOT(slotSelectAll()));

    GuiWindowMain::createToolButton(&selectInvertButton_,
                                    tr("Invert"),
                                    tr("Invert selection"),
                                    ICON("select_invert"),
                                    this,
                                    SLOT(slotSelectInvert()));

    GuiWindowMain::createToolButton(&selectNoneButton_,
                                    tr("Select none"),
                                    tr("Select none"),
                                    ICON("select_none"),
                                    this,
                                    SLOT(slotSelectNone()));

    // Tool bar
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(addButton_);
    toolBar->addWidget(deleteButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(
        QSize(GuiWindowMain::ICON_SIZE, GuiWindowMain::ICON_SIZE));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);
    setLayout(mainLayout);

    // Data
    connect(mainWindow_, SIGNAL(signalUpdate()), this, SLOT(slotUpdate()));
}

void GuiProjectDatasets::dataChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasets(datasets_);
    mainWindow_->updateData();
}

void GuiProjectDatasets::selectionChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasets(datasets_);
    mainWindow_->updateSelection();
}

void GuiProjectDatasets::slotUpdate()
{
    setDatasets(mainWindow_->editor().datasets());
}

void GuiProjectDatasets::slotAdd()
{
    GuiPluginImport::import(mainWindow_);
}

void GuiProjectDatasets::slotDelete()
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

    dataChanged();
}

void GuiProjectDatasets::slotSelectAll()
{
    datasets_.setEnabledAll(true);
    updateTree();
    selectionChanged();
}

void GuiProjectDatasets::slotSelectInvert()
{
    datasets_.setInvertAll();
    updateTree();
    selectionChanged();
}

void GuiProjectDatasets::slotSelectNone()
{
    datasets_.setEnabledAll(false);
    updateTree();
    selectionChanged();
}

void GuiProjectDatasets::slotItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        deleteButton_->setEnabled(true);
    }
    else
    {
        deleteButton_->setEnabled(false);
    }
}

void GuiProjectDatasets::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        datasets_.setEnabled(index(item), checked);
        selectionChanged();
    }
}

size_t GuiProjectDatasets::index(const QTreeWidgetItem *item)
{
    return datasets_.index(item->text(COLUMN_ID).toULong());
}

void GuiProjectDatasets::updateTree()
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

void GuiProjectDatasets::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void GuiProjectDatasets::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    connect(tree_,
            SIGNAL(itemSelectionChanged()),
            this,
            SLOT(slotItemSelectionChanged()));
}

void GuiProjectDatasets::addItem(size_t i)
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

void GuiProjectDatasets::setDatasets(const EditorDatasets &datasets)
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