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

/** @file ProjectNavigatorClassifications.cpp */

#include <ColorPalette.hpp>

#include <MainWindow.hpp>
#include <ProjectNavigatorClassifications.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

ProjectNavigatorClassifications::ProjectNavigatorClassifications(
    MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    // Table
    tree_ = new QTreeWidget();
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Tool bar buttons
    MainWindow::createToolButton(&showButton_,
                                 tr("Show"),
                                 tr("Make selected classifications visible"),
                                 ICON("eye"),
                                 this,
                                 SLOT(slotShow()));
    showButton_->setEnabled(false);

    MainWindow::createToolButton(&hideButton_,
                                 tr("Hide"),
                                 tr("Hide selected classifications"),
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
    toolBar->addWidget(showButton_);
    toolBar->addWidget(hideButton_);
    toolBar->addSeparator();
    toolBar->addWidget(selectAllButton_);
    toolBar->addWidget(selectInvertButton_);
    toolBar->addWidget(selectNoneButton_);
    toolBar->setIconSize(QSize(MainWindow::ICON_SIZE, MainWindow::ICON_SIZE));

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tree_);
    setLayout(mainLayout);

    // Data
    connect(mainWindow_, SIGNAL(signalUpdate()), this, SLOT(slotUpdate()));
}

void ProjectNavigatorClassifications::dataChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setClassifications(classifications_);
    mainWindow_->updateData();
}

void ProjectNavigatorClassifications::filterChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setClassifications(classifications_);
    mainWindow_->updateFilter();
}

void ProjectNavigatorClassifications::slotUpdate()
{
    setClassifications(mainWindow_->editor().classifications());
}

void ProjectNavigatorClassifications::slotShow()
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

void ProjectNavigatorClassifications::slotHide()
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

void ProjectNavigatorClassifications::slotSelectAll()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(true);
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorClassifications::slotSelectInvert()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(!(*it)->isSelected());
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorClassifications::slotSelectNone()
{
    QTreeWidgetItemIterator it(tree_);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    slotItemSelectionChanged();
}

void ProjectNavigatorClassifications::slotItemSelectionChanged()
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

void ProjectNavigatorClassifications::slotItemChanged(QTreeWidgetItem *item,
                                                      int column)
{
    if (column == COLUMN_CHECKED)
    {
        size_t id = item->text(COLUMN_ID).toULong();
        bool checked = (item->checkState(COLUMN_CHECKED) == Qt::Checked);

        classifications_.setEnabled(id, checked);
        filterChanged();
    }
}

void ProjectNavigatorClassifications::updateTree()
{
    block();

    QTreeWidgetItemIterator it(tree_);
    size_t i = 0;

    while (*it)
    {
        if (classifications_.isEnabled(i))
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

void ProjectNavigatorClassifications::block()
{
    disconnect(tree_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(tree_, SIGNAL(itemSelectionChanged()), 0, 0);
    (void)blockSignals(true);
}

void ProjectNavigatorClassifications::unblock()
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

void ProjectNavigatorClassifications::addItem(size_t i)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree_);

    if (classifications_.isEnabled(i))
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Checked);
    }
    else
    {
        item->setCheckState(COLUMN_CHECKED, Qt::Unchecked);
    }

    item->setText(COLUMN_ID, QString::number(i));

    item->setText(COLUMN_LABEL,
                  QString::fromStdString(classifications_.label(i)));

    // Color legend
    if (i < ColorPalette::Classification.size())
    {
        const Vector3<float> &rgb = ColorPalette::Classification[i];

        QColor color;
        color.setRedF(rgb[0]);
        color.setGreenF(rgb[1]);
        color.setBlueF(rgb[2]);

        QBrush brush(color, Qt::SolidPattern);
        item->setBackground(COLUMN_ID, brush);
        // brush.setColor(QColor(0, 0, 0));
        // item->setForeground(COLUMN_ID, brush);
    }
}

void ProjectNavigatorClassifications::setClassifications(
    const Classifications &classifications)
{
    block();

    classifications_ = classifications;

    tree_->clear();

    // Header
    tree_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Visible") << tr("Class") << tr("Label");
    tree_->setHeaderLabels(labels);

    // Content
    for (size_t i = 0; i < classifications_.size(); i++)
    {
        addItem(i);
    }

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        tree_->resizeColumnToContents(i);
    }

    unblock();
}
