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

/** @file FilterClassificationWidget.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <FilterClassificationWidget.hpp>
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
#define LOG_MODULE_NAME "FilterClassificationWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterClassificationResources/", name))

FilterClassificationWidget::FilterClassificationWidget(MainWindow *mainWindow)
    : mainWindow_(mainWindow)
{
    // Table.
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons.
    MainWindow::createToolButton(&showButton_,
                                 tr("Show"),
                                 tr("Make selected classifications visible"),
                                 THEME_ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected classifications"),
                                 THEME_ICON("hide"),
                                 this,
                                 SLOT(slotHide()));
    hideButton_->setEnabled(false);

    MainWindow::createToolButton(&selectAllButton_,
                                 tr("Select all"),
                                 tr("Select all"),
                                 THEME_ICON("select-all"),
                                 this,
                                 SLOT(slotSelectAll()));

    MainWindow::createToolButton(&selectInvertButton_,
                                 tr("Invert"),
                                 tr("Invert selection"),
                                 THEME_ICON("select-invert"),
                                 this,
                                 SLOT(slotSelectInvert()));

    MainWindow::createToolButton(&selectNoneButton_,
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

void FilterClassificationWidget::slotUpdate(void *sender,
                                            const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_CLASSIFICATION))
    {
        LOG_DEBUG_UPDATE(<< "Input classifications.");

        setClassifications(mainWindow_->editor().classifications(),
                           mainWindow_->editor().classificationsFilter());
    }
}

void FilterClassificationWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Output classifications.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClassifications(classifications_);
    mainWindow_->editor().setClassificationsFilter(filter_);
    mainWindow_->updateData();
}

void FilterClassificationWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Output classifications filter.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClassificationsFilter(filter_);
    mainWindow_->updateFilter();
}

void FilterClassificationWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set classifications filer enabled <" << toString(b) << ">.");

    filter_.setEnabled(b);
    filterChanged();
}

void FilterClassificationWidget::setClassifications(
    const Classifications &classifications,
    const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set classifications n <" << classifications.size() << ">.");

    block();

    classifications_ = classifications;
    filter_ = filter;

    tree_->clear();

    // Header.
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Class") << tr("Label");
    tree_->setHeaderLabels(labels);

    // Content.
    for (size_t i = 0; i < classifications_.size(); i++)
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

void FilterClassificationWidget::slotShow()
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

void FilterClassificationWidget::slotHide()
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

void FilterClassificationWidget::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void FilterClassificationWidget::slotItemSelectionChanged()
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

void FilterClassificationWidget::slotItemChanged(QTreeWidgetItem *item,
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

size_t FilterClassificationWidget::identifier(const QTreeWidgetItem *item)
{
    return static_cast<size_t>(item->text(COLUMN_ID).toULong());
}

void FilterClassificationWidget::updateTree()
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

void FilterClassificationWidget::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void FilterClassificationWidget::unblock()
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

void FilterClassificationWidget::addTreeItem(size_t index)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (filter_.enabled(index))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(index));

    item->setText(COLUMN_LABEL,
                  QString::fromStdString(classifications_.label(index)));

    // Color legend.
    if (index < ColorPalette::Classification.size())
    {
        const Vector3<double> &rgb = ColorPalette::Classification[index];

        QColor color;
        color.setRedF(static_cast<float>(rgb[0]));
        color.setGreenF(static_cast<float>(rgb[1]));
        color.setBlueF(static_cast<float>(rgb[2]));

        QBrush brush(color, Qt::SolidPattern);
        item->setBackground(COLUMN_ID, brush);
        // brush.setColor(QColor(0, 0, 0));
        // item->setForeground(COLUMN_ID, brush);
    }
}
