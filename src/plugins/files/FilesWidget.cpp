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

/** @file FilesWidget.cpp */

// Include 3D Forest.
#include <FilesWidget.hpp>
#include <ImportFilePlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilesWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/files/", name))

FilesWidget::FilesWidget(MainWindow *mainWindow) : mainWindow_(mainWindow)
{
    // Table.
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons.
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
                                 THEME_ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected data sets"),
                                 THEME_ICON("hide"),
                                 this,
                                 SLOT(slotHide()));
    hideButton_->setEnabled(false);

    MainWindow::createToolButton(&selectAllButton_,
                                 tr("Select all"),
                                 tr("Select all"),
                                 THEME_ICON("select_all"),
                                 this,
                                 SLOT(slotSelectAll()));

    MainWindow::createToolButton(&selectInvertButton_,
                                 tr("Invert"),
                                 tr("Invert selection"),
                                 THEME_ICON("select_invert"),
                                 this,
                                 SLOT(slotSelectInvert()));

    MainWindow::createToolButton(&selectNoneButton_,
                                 tr("Select none"),
                                 tr("Select none"),
                                 THEME_ICON("select_none"),
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

void FilesWidget::slotUpdate(void *sender, const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_DATA_SET))
    {
        LOG_DEBUG_UPDATE(<< "Input datasets.");

        setDatasets(mainWindow_->editor().datasets(),
                    mainWindow_->editor().datasetsFilter());
    }
}

void FilesWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output datasets.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasets(datasets_);
    mainWindow_->editor().setDatasetsFilter(filter_);
    mainWindow_->updateData();
}

void FilesWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output datasets filter <" << filter_.enabled()
                     << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDatasetsFilter(filter_);
    mainWindow_->updateFilter();
}

void FilesWidget::setDatasets(const Datasets &datasets,
                              const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set datasets n <" << datasets.size() << ">.");

    block();

    datasets_ = datasets;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Id") << tr("Label") << tr("Date");
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < datasets_.size(); i++)
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

void FilesWidget::slotAdd()
{
    ImportFilePlugin::import(mainWindow_);
}

void FilesWidget::slotDelete()
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

void FilesWidget::slotShow()
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

void FilesWidget::slotHide()
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

void FilesWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilesWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void FilesWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilesWidget::slotItemSelectionChanged()
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

void FilesWidget::slotItemChanged(QTreeWidgetItem *item, int column)
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

size_t FilesWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

size_t FilesWidget::index(const QTreeWidgetItem *item)
{
    return datasets_.index(item->text(COLUMN_ID).toULong());
}

void FilesWidget::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        size_t id = identifier(*it);

        if (filter_.enabled(id))
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

void FilesWidget::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void FilesWidget::unblock()
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

void FilesWidget::addTreeItem(size_t index)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    size_t id = datasets_.id(index);

    if (filter_.enabled(id))
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

    // Color legend.
    const Vector3<double> &rgb = datasets_.color(index);

    QColor color;
    color.setRedF(static_cast<float>(rgb[0]));
    color.setGreenF(static_cast<float>(rgb[1]));
    color.setBlueF(static_cast<float>(rgb[2]));

    QBrush brush(color, Qt::SolidPattern);
    item->setBackground(COLUMN_ID, brush);
}
