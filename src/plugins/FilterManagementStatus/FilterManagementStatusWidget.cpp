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

/** @file FilterManagementStatusWidget.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <FilterManagementStatusTreeWidget.hpp>
#include <FilterManagementStatusWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterManagementStatusWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterManagementStatusResources/", name))

FilterManagementStatusWidget::FilterManagementStatusWidget(
    MainWindow *mainWindow)
    : mainWindow_(mainWindow)
{
    // Table.
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons.
    mainWindow_->createToolButton(&showButton_,
                                  tr("Show"),
                                  tr("Make selected management status visible"),
                                  THEME_ICON("eye"),
                                  this,
                                  SLOT(slotShow()));
    showButton_->setEnabled(false);

    mainWindow_->createToolButton(&hideButton_,
                                  tr("Hide"),
                                  tr("Hide selected management status"),
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

    // Detail.
    treeWidget_ = new FilterManagementStatusTreeWidget(mainWindow_);

    // Splitter.
    splitter_ = new QSplitter;
    splitter_->addWidget(tree_);
    splitter_->addWidget(treeWidget_);
    splitter_->setOrientation(Qt::Vertical);
    splitter_->setSizes(QList<int>({1, 1}));

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(splitter_);

    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());
}

void FilterManagementStatusWidget::slotUpdate(void *sender,
                                              const QSet<Editor::Type> &target)
{
    if (sender == this || sender == treeWidget_)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_MANAGEMENT_STATUS))
    {
        LOG_DEBUG_UPDATE(<< "Input management status.");

        setManagementStatusList(mainWindow_->editor().managementStatusList(),
                                mainWindow_->editor().managementStatusFilter());
    }

    if (target.empty() || target.contains(Editor::TYPE_SEGMENT))
    {
        LOG_DEBUG_UPDATE(<< "Input segment.");
        bool found = false;
        const Segments &segments = mainWindow_->editor().segments();
        for (size_t i = 0; i < segments.size(); i++)
        {
            if (segments[i].selected)
            {
                treeWidget_->setSegment(segments[i]);
                found = true;
                break;
            }
        }
        if (!found)
        {
            treeWidget_->clear();
        }
    }
}

void FilterManagementStatusWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output management status.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setManagementStatusList(managementStatus_);
    mainWindow_->editor().setManagementStatusFilter(filter_);
    mainWindow_->updateData();
}

void FilterManagementStatusWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output management status filter.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setManagementStatusFilter(filter_);
    mainWindow_->updateFilter();
}

void FilterManagementStatusWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set management status filer enabled <" << toString(b)
              << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterManagementStatusWidget::setManagementStatusList(
    const ManagementStatusList &managementStatus,
    const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set management status n <" << managementStatus.size()
              << ">.");

    block();

    managementStatus_ = managementStatus;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Name");
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < managementStatus_.size(); i++)
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

void FilterManagementStatusWidget::slotShow()
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

void FilterManagementStatusWidget::slotHide()
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

void FilterManagementStatusWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterManagementStatusWidget::slotItemSelectionChanged()
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

void FilterManagementStatusWidget::slotItemChanged(QTreeWidgetItem *item,
                                                   int column)
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

size_t FilterManagementStatusWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

void FilterManagementStatusWidget::updateTree()
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

void FilterManagementStatusWidget::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void FilterManagementStatusWidget::unblock()
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

void FilterManagementStatusWidget::addTreeItem(size_t index)
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
    const ManagementStatus &managementStatus = managementStatus_[index];

    // Id.
    item->setText(COLUMN_ID, QString::number(managementStatus.id));

    // Label.
    item->setText(COLUMN_LABEL, QString::fromStdString(managementStatus.label));

    // Color legend.
    QColor color;
    color.setRedF(static_cast<float>(managementStatus.color[0]));
    color.setGreenF(static_cast<float>(managementStatus.color[1]));
    color.setBlueF(static_cast<float>(managementStatus.color[2]));

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}
