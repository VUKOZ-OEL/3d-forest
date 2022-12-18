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

/** @file ProjectNavigatorItemLayers.cpp */

#include <ImportFilePlugin.hpp>
#include <Log.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorItemLayers.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorItemLayers::ProjectNavigatorItemLayers(MainWindow *mainWindow,
                                                       const QIcon &icon,
                                                       const QString &text)
    : ProjectNavigatorItem(mainWindow, icon, text)
{
    // Table
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons
    MainWindow::createToolButton(&addButton_,
                                 tr("Add"),
                                 tr("Add new layers"),
                                 THEME_ICON("add"),
                                 this,
                                 SLOT(slotAdd()));
    addButton_->setEnabled(false);

    MainWindow::createToolButton(&deleteButton_,
                                 tr("Remove"),
                                 tr("Remove selected layers"),
                                 THEME_ICON("remove"),
                                 this,
                                 SLOT(slotDelete()));
    deleteButton_->setEnabled(false);

    MainWindow::createToolButton(&showButton_,
                                 tr("Show"),
                                 tr("Make selected layers visible"),
                                 ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected layers"),
                                 ICON("hide"),
                                 this,
                                 SLOT(slotHide()));
    hideButton_->setEnabled(false);

    MainWindow::createToolButton(&selectAllButton_,
                                 tr("Select all"),
                                 tr("Select all"),
                                 ICON("select_all"),
                                 this,
                                 SLOT(slotSelectAll()));

    MainWindow::createToolButton(&selectInvertButton_,
                                 tr("Invert"),
                                 tr("Invert selection"),
                                 ICON("select_invert"),
                                 this,
                                 SLOT(slotSelectInvert()));

    MainWindow::createToolButton(&selectNoneButton_,
                                 tr("Select none"),
                                 tr("Select none"),
                                 ICON("select_none"),
                                 this,
                                 SLOT(slotSelectNone()));

    // Tool bar
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(addButton_);
    toolBar->addWidget(deleteButton_);
    toolBar->addWidget(showButton_);
    toolBar->addWidget(hideButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(QSize(MainWindow::ICON_SIZE, MainWindow::ICON_SIZE));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(const QSet<Editor::Type> &)));
}

void ProjectNavigatorItemLayers::slotUpdate(const QSet<Editor::Type> &target)
{
    if (!target.empty() && !target.contains(Editor::TYPE_LAYER))
    {
        return;
    }

    setLayers(mainWindow_->editor().layers());
}

void ProjectNavigatorItemLayers::dataChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setLayers(layers_);
    mainWindow_->updateData();
}

void ProjectNavigatorItemLayers::filterChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setLayers(layers_);
    mainWindow_->updateFilter();
}

void ProjectNavigatorItemLayers::slotAdd()
{
}

void ProjectNavigatorItemLayers::slotDelete()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        slotSelectNone();

        for (auto &item : items)
        {
            size_t idx = index(item);
            layers_.erase(idx);

            delete item;
        }

        dataChanged();
    }
}

void ProjectNavigatorItemLayers::slotShow()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }

        filterChanged();
    }
}

void ProjectNavigatorItemLayers::slotHide()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }

        filterChanged();
    }
}

void ProjectNavigatorItemLayers::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorItemLayers::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorItemLayers::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorItemLayers::slotItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        deleteButton_->setEnabled(true);
        showButton_->setEnabled(true);
        hideButton_->setEnabled(true);
    }
    else
    {
        deleteButton_->setEnabled(false);
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
    }
}

void ProjectNavigatorItemLayers::slotItemChanged(QTreeWidgetItem *item,
                                                 int column)
{
    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        layers_.setEnabled(index(item), checked);
        filterChanged();
    }
}

size_t ProjectNavigatorItemLayers::index(const QTreeWidgetItem *item)
{
    return layers_.index(item->text(COLUMN_ID).toULong());
}

void ProjectNavigatorItemLayers::updateTree()
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

void ProjectNavigatorItemLayers::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void ProjectNavigatorItemLayers::unblock()
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

void ProjectNavigatorItemLayers::addItem(size_t i)
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

void ProjectNavigatorItemLayers::setLayers(const Layers &layers)
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

    // Sort Content
    tree_->setSortingEnabled(true);
    tree_->sortItems(COLUMN_ID, Qt::AscendingOrder);

    unblock();
}