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

/** @file AlgorithmTabWidget.cpp */

#include <AlgorithmTabWidget.hpp>
#include <MainWindow.hpp>

#include <QFrame>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "AlgorithmTabWidget"
#include <Log.hpp>

AlgorithmTabWidget::AlgorithmTabWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      treeWidget_(nullptr),
      widgetsLayout_(nullptr)
{
    LOG_DEBUG(<< "Create algorithm tab widget.");

    // Create the tree widget.
    treeWidget_ = new QTreeWidget();

    treeWidget_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Algorithm") << tr("Id");
    treeWidget_->setHeaderLabels(labels);
    treeWidget_->setColumnHidden(COLUMN_ID, true);

    // Resize the columns to the minimum space.
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        treeWidget_->resizeColumnToContents(i);
    }

    // Create the tree detail view.
    widgetsLayout_ = new QVBoxLayout;
    widgetsLayout_->setContentsMargins(0, 0, 0, 0);

    QFrame *treeDetailFrame = new QFrame;
    treeDetailFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
    treeDetailFrame->setLineWidth(0);
    treeDetailFrame->setContentsMargins(0, 0, 0, 0);
    treeDetailFrame->setLayout(widgetsLayout_);

    // Create splitter layout between the tree and the detail view.
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(treeWidget_);
    splitter->addWidget(treeDetailFrame);

    int w = width() / 4;
    splitter->setSizes(QList<int>({w * 1, w * 3}));

    // Setup the main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(1, 1, 1, 1);

    setLayout(mainLayout);
}

void AlgorithmTabWidget::addItem(AlgorithmWidgetInterface *widget)
{
    LOG_DEBUG(<< "Add widget text <" << widget->text().toStdString() << ">.");

    block();

    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_);

    item->setText(COLUMN_ID, QString::number(widgets_.size()));
    item->setIcon(COLUMN_LABEL, widget->icon());
    item->setText(COLUMN_LABEL, widget->text());

    // Register new widget.
    widgets_.push_back(widget);

    if (widgets_.size() == 1)
    {
        widget->setVisible(true);
    }
    else
    {
        widget->setVisible(false);
    }

    widgetsLayout_->addWidget(widget);

    unblock();
}

void AlgorithmTabWidget::slotItemClicked(QTreeWidgetItem *item, int column)
{
    if (item == nullptr || column < 0)
    {
        return;
    }

    if (column == COLUMN_LABEL)
    {
        setTabVisible(index(item));
    }
}

size_t AlgorithmTabWidget::index(const QTreeWidgetItem *item)
{
    return item->text(COLUMN_ID).toULong();
}

void AlgorithmTabWidget::block()
{
    disconnect(treeWidget_, SIGNAL(itemClicked(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void AlgorithmTabWidget::unblock()
{
    (void)blockSignals(false);
    connect(treeWidget_,
            SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemClicked(QTreeWidgetItem *, int)));
}

void AlgorithmTabWidget::setTabVisible(size_t index)
{
    LOG_DEBUG(<< "Called with index <" << index << ">.");

    for (size_t i = 0; i < widgets_.size(); i++)
    {
        if (i != index)
        {
            LOG_DEBUG(<< "Hide widget <" << i << ">.");
            widgets_[i]->setVisible(false);
        }
    }

    if (index < widgets_.size())
    {
        LOG_DEBUG(<< "Show widget <" << index << ">.");
        widgets_[index]->setVisible(true);
    }
}
