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

/** @file ExplorerSegmentsWidget.cpp */

// Include 3D Forest.
#include <ExplorerSegmentWidget.hpp>
#include <ExplorerSegmentsWidget.hpp>
#include <ImportFilePlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerSegmentsWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/explorer/", name))

ExplorerSegmentsWidget::ExplorerSegmentsWidget(MainWindow *mainWindow,
                                               const QIcon &icon,
                                               const QString &text)
    : ExplorerWidgetInterface(mainWindow, icon, text)
{
    // Table.
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons.
    MainWindow::createToolButton(&addButton_,
                                 tr("Add"),
                                 tr("Add new segments"),
                                 THEME_ICON("add"),
                                 this,
                                 SLOT(slotAdd()));
    addButton_->setEnabled(false);

    MainWindow::createToolButton(&deleteButton_,
                                 tr("Remove"),
                                 tr("Remove selected segments"),
                                 THEME_ICON("remove"),
                                 this,
                                 SLOT(slotDelete()));
    deleteButton_->setEnabled(false);

    MainWindow::createToolButton(&showButton_,
                                 tr("Show"),
                                 tr("Make selected segments visible"),
                                 ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected segments"),
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

    // Tool bar.
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

    // Segment.
    segmentWidget_ = new ExplorerSegmentWidget;

    // Splitter.
    splitter_ = new QSplitter;
    splitter_->addWidget(tree_);
    splitter_->addWidget(segmentWidget_);
    splitter_->setOrientation(Qt::Vertical);
    splitter_->setSizes(QList<int>({1, 1}));

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(splitter_);

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data.
    updatesEnabled_ = true;
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ExplorerSegmentsWidget::slotUpdate(void *sender,
                                        const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SEGMENT))
    {
        setSegments(mainWindow_->editor().segments(),
                    mainWindow_->editor().segmentsFilter());
    }
}

void ExplorerSegmentsWidget::setSegments(const Segments &segments,
                                         const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Input segments <" << segments.size() << ">.");

    block();

    segments_ = segments;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Label");
    tree_->setHeaderLabels(labels);

    if (segments_.size() > 0)
    {
        segmentWidget_->setSegment(segments_[0]);
    }
    else
    {
        segmentWidget_->clear();
    }

    // Content.
    for (size_t i = 0; i < segments_.size(); i++)
    {
        addTreeItem(i);
    }

    // Resize Columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    // Sort Content.
    tree_->setSortingEnabled(true);
    tree_->sortItems(COLUMN_ID, Qt::AscendingOrder);

    unblock();
}

void ExplorerSegmentsWidget::dataChanged()
{
    LOG_DEBUG(<< "Output segments <" << segments_.size() << ">.");
    LOG_DEBUG(<< "Output segments filter <" << filter_.isFilterEnabled()
              << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSegments(segments_);
    mainWindow_->editor().setSegmentsFilter(filter_);
    mainWindow_->updateData();
}

void ExplorerSegmentsWidget::filterChanged()
{
    LOG_DEBUG(<< "Output segments filter <" << filter_.isFilterEnabled()
              << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSegmentsFilter(filter_);
    mainWindow_->updateFilter();
}

bool ExplorerSegmentsWidget::isFilterEnabled() const
{
    return filter_.isFilterEnabled();
}

void ExplorerSegmentsWidget::setFilterEnabled(bool b)
{
    filter_.setFilterEnabled(b);
    filterChanged();
}

void ExplorerSegmentsWidget::slotAdd()
{
}

void ExplorerSegmentsWidget::slotDelete()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        slotSelectNone();

        for (auto &item : items)
        {
            segments_.erase(index(item));
            filter_.erase(identifier(item));

            delete item;
        }

        dataChanged();
    }
}

void ExplorerSegmentsWidget::slotShow()
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

void ExplorerSegmentsWidget::slotHide()
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

void ExplorerSegmentsWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerSegmentsWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerSegmentsWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerSegmentsWidget::slotItemSelectionChanged()
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

void ExplorerSegmentsWidget::slotItemClicked(QTreeWidgetItem *item, int column)
{
    if (!item)
    {
        return;
    }

    size_t id = identifier(item);
    size_t index = segments_.index(id);
    LOG_DEBUG(<< "item ID <" << id << "> index <" << index << "> column <"
              << column << ">.");

    if (column == COLUMN_CHECKED)
    {
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        filter_.setFilter(id, checked);

        if (updatesEnabled_)
        {
            filterChanged();
        }

        return;
    }

    segmentWidget_->setSegment(segments_[index]);

    for (size_t i = 0; i < segments_.size(); i++)
    {
        segments_[i].selected = false;
    }
    segments_[index].selected = true;

    if (updatesEnabled_)
    {
        dataChanged();
    }
}

void ExplorerSegmentsWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (!item)
    {
        return;
    }

    (void)column;
}

size_t ExplorerSegmentsWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

size_t ExplorerSegmentsWidget::index(const QTreeWidgetItem *item)
{
    return segments_.index(item->text(COLUMN_ID).toULong());
}

void ExplorerSegmentsWidget::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t id = identifier(*it);

        if (filter_.hasFilter(id))
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

void ExplorerSegmentsWidget::block()
{
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    disconnect(tree_, SIGNAL(itemClicked(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void ExplorerSegmentsWidget::unblock()
{
    (void)blockSignals(false);
    connect(tree_,
            SIGNAL(itemSelectionChanged()),
            this,
            SLOT(slotItemSelectionChanged()));
    connect(tree_,
            SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemClicked(QTreeWidgetItem *, int)));
    connect(tree_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemChanged(QTreeWidgetItem *, int)));
}

void ExplorerSegmentsWidget::addTreeItem(size_t index)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    size_t id = segments_.id(index);

    if (filter_.hasFilter(id))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(id));

    item->setText(COLUMN_LABEL, QString::fromStdString(segments_[index].label));

    // Color legend.
    const Vector3<double> &rgb = segments_[index].color;

    QColor color;
    color.setRedF(static_cast<float>(rgb[0]));
    color.setGreenF(static_cast<float>(rgb[1]));
    color.setBlueF(static_cast<float>(rgb[2]));

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}
