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

/** @file FilterSpeciesWidget.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <FilterSpeciesWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterSpeciesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterSpeciesResources/", name))

FilterSpeciesWidget::FilterSpeciesWidget(MainWindow *mainWindow)
    : mainWindow_(mainWindow)
{
    // Table.
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons.
    mainWindow_->createToolButton(&showButton_,
                                  tr("Show"),
                                  tr("Make selected species visible"),
                                  THEME_ICON("eye"),
                                  this,
                                  SLOT(slotShow()));
    showButton_->setEnabled(false);

    mainWindow_->createToolButton(&hideButton_,
                                  tr("Hide"),
                                  tr("Hide selected species"),
                                  THEME_ICON("hide"),
                                  this,
                                  SLOT(slotHide()));
    hideButton_->setEnabled(false);

    mainWindow_->createToolButton(&selectAllButton_,
                                  tr("Select all"),
                                  tr("Select all"),
                                  THEME_ICON("select-all"),
                                  this,
                                  SLOT(slotSelectAll()));

    mainWindow_->createToolButton(&selectInvertButton_,
                                  tr("Invert"),
                                  tr("Invert selection"),
                                  THEME_ICON("select-invert"),
                                  this,
                                  SLOT(slotSelectInvert()));

    mainWindow_->createToolButton(&selectNoneButton_,
                                  tr("Select none"),
                                  tr("Select none"),
                                  THEME_ICON("select-none"),
                                  this,
                                  SLOT(slotSelectNone()));

    // Tool bar.
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(showButton_);
    toolBar->addWidget(hideButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(QSize(MainWindow::ICON_SIZE, MainWindow::ICON_SIZE));

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);

    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());
}

void FilterSpeciesWidget::slotUpdate(void *sender,
                                     const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SPECIES))
    {
        LOG_DEBUG_UPDATE(<< "Input species.");

        setSpeciesList(mainWindow_->editor().speciesList(),
                       mainWindow_->editor().speciesFilter());
    }
}

void FilterSpeciesWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output species.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSpeciesList(species_);
    mainWindow_->editor().setSpeciesFilter(filter_);
    mainWindow_->updateData();
}

void FilterSpeciesWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output species filter.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSpeciesFilter(filter_);
    mainWindow_->updateFilter();
}

void FilterSpeciesWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set species filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterSpeciesWidget::setSpeciesList(const SpeciesList &species,
                                         const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set species n <" << species.size() << ">.");

    block();

    species_ = species;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Name");
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < species_.size(); i++)
    {
        addTreeItem(i);
    }

    // Resize Columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    unblock();
}

void FilterSpeciesWidget::slotShow()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        updatesEnabled_ = true;

        filterChanged();
    }
}

void FilterSpeciesWidget::slotHide()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        updatesEnabled_ = false;
        for (auto &item : items)
        {
            item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }
        updatesEnabled_ = true;

        filterChanged();
    }
}

void FilterSpeciesWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterSpeciesWidget::slotItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        showButton_->setEnabled(true);
        hideButton_->setEnabled(true);
    }
    else
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
    }
}

void FilterSpeciesWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = identifier(item);
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        filter_.setEnabled(id, checked);

        if (updatesEnabled_)
        {
            filterChanged();
        }
    }
}

size_t FilterSpeciesWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

void FilterSpeciesWidget::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);
    size_t i = 0;

    while (*it)
    {
        if (filter_.enabled(i))
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Checked);
        }
        else
        {
            (*it)->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
        }

        ++i;
        ++it;
    }

    unblock();
}

void FilterSpeciesWidget::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void FilterSpeciesWidget::unblock()
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

void FilterSpeciesWidget::addTreeItem(size_t index)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    // Checked.
    if (filter_.enabled(index))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    // Data.
    const Species &species = species_[index];

    // Id.
    item->setText(COLUMN_ID, QString::number(species.id));

    // Label.
    item->setText(COLUMN_LABEL, QString::fromStdString(species.latin));

    // Color legend.
    QColor color;
    color.setRedF(static_cast<float>(species.color[0]));
    color.setGreenF(static_cast<float>(species.color[1]));
    color.setBlueF(static_cast<float>(species.color[2]));

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}
