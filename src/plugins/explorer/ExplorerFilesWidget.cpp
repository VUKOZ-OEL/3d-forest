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

/** @file ExplorerFilesWidget.cpp */

#include <ExplorerFilesWidget.hpp>
#include <ImportFilePlugin.hpp>
#include <MainWindow.hpp>
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

#define LOG_MODULE_NAME "ExplorerFilesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/explorer/", name))

ExplorerFilesWidget::ExplorerFilesWidget(MainWindow *mainWindow,
                                         const QIcon &icon,
                                         const QString &text)
    : ExplorerWidgetInterface(mainWindow, icon, text)
{
    // Table
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons
    MainWindow::createToolButton(&addButton_,
                                 tr("Add"),
                                 tr("Add new data set"),
                                 THEME_ICON("add"),
                                 this,
                                 SLOT(slotAdd()));

    MainWindow::createToolButton(&deleteButton_,
                                 tr("Remove"),
                                 tr("Remove selected data set"),
                                 THEME_ICON("remove"),
                                 this,
                                 SLOT(slotDelete()));
    deleteButton_->setEnabled(false);

    MainWindow::createToolButton(&showButton_,
                                 tr("Show"),
                                 tr("Make selected data sets visible"),
                                 ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected data sets"),
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
    updatesEnabled_ = true;
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ExplorerFilesWidget::slotUpdate(void *sender,
                                     const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_DATA_SET))
    {
        setDatasets(mainWindow_->editor().datasets(),
                    mainWindow_->editor().datasetsFilter());
    }
}

void ExplorerFilesWidget::setDatasets(const Datasets &datasets,
                                      const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Input datasets <" << datasets.size() << ">.");

    block();

    datasets_ = datasets;
    filter_ = filter;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Label") << tr("Date");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < datasets_.size(); i++)
    {
        addTreeItem(i);
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

void ExplorerFilesWidget::dataChanged()
{
    LOG_DEBUG(<< "Output datasets <" << datasets_.size() << ">.");
    LOG_DEBUG(<< "Output datasets filter <" << filter_.isFilterEnabled()
              << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasets(datasets_);
    mainWindow_->editor().setDatasetsFilter(filter_);
    mainWindow_->updateData();
}

void ExplorerFilesWidget::filterChanged()
{
    LOG_DEBUG(<< "Output datasets filter <" << filter_.isFilterEnabled()
              << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasetsFilter(filter_);
    mainWindow_->updateFilter();
}

bool ExplorerFilesWidget::isFilterEnabled() const
{
    return filter_.isFilterEnabled();
}

void ExplorerFilesWidget::setFilterEnabled(bool b)
{
    filter_.setFilterEnabled(b);
    filterChanged();
}

void ExplorerFilesWidget::slotAdd()
{
    ImportFilePlugin::import(mainWindow_);
}

void ExplorerFilesWidget::slotDelete()
{
    QList<QTreeWidgetItem *> items = tree_->selectedItems();

    if (items.count() > 0)
    {
        slotSelectNone();

        for (auto &item : items)
        {
            datasets_.erase(index(item));
            filter_.erase(identifier(item));

            delete item;
        }

        dataChanged();
    }
}

void ExplorerFilesWidget::slotShow()
{
    LOG_DEBUG(<< "Show.");
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

void ExplorerFilesWidget::slotHide()
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

void ExplorerFilesWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerFilesWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerFilesWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void ExplorerFilesWidget::slotItemSelectionChanged()
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

void ExplorerFilesWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = identifier(item);
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        filter_.setFilter(id, checked);

        if (updatesEnabled_)
        {
            filterChanged();
        }
    }
}

size_t ExplorerFilesWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

size_t ExplorerFilesWidget::index(const QTreeWidgetItem *item)
{
    return datasets_.index(item->text(COLUMN_ID).toULong());
}

void ExplorerFilesWidget::updateTree()
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

void ExplorerFilesWidget::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void ExplorerFilesWidget::unblock()
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

void ExplorerFilesWidget::addTreeItem(size_t index)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    size_t id = datasets_.id(index);

    if (filter_.hasFilter(id))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(id));

    item->setText(COLUMN_LABEL, QString::fromStdString(datasets_.label(index)));
    item->setText(COLUMN_DATE_CREATED,
                  QString::fromStdString(datasets_.dateCreated(index)));

    // Color legend
    const Vector3<double> &rgb = datasets_.color(index);

    QColor color;
    color.setRedF(static_cast<float>(rgb[0]));
    color.setGreenF(static_cast<float>(rgb[1]));
    color.setBlueF(static_cast<float>(rgb[2]));

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}
