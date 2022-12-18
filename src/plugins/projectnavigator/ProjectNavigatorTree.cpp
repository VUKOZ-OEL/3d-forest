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

/** @file ProjectNavigatorTree.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorTree.hpp>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

#define MODULE_NAME "ProjectNavigatorTree"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ProjectNavigatorTree::ProjectNavigatorTree(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      treeWidget_(nullptr),
      tabLayout_(nullptr)
{
    LOG_DEBUG_LOCAL("");

    // Tree
    treeWidget_ = new QTreeWidget();

    treeWidget_->setColumnCount(COLUMN_LAST);
    QStringList labels;
    labels << tr("Color") << tr("Filter") << tr("Item") << tr("Id");
    treeWidget_->setHeaderLabels(labels);
    treeWidget_->setColumnHidden(COLUMN_ID, true);

    // Resize Columns to the minimum space
    for (int i = 0; i < COLUMN_LAST; i++)
    {
        treeWidget_->resizeColumnToContents(i);
    }

    // Tree detail
    tabLayout_ = new QVBoxLayout;
    tabLayout_->setContentsMargins(0, 0, 0, 0);

    QFrame *treeDetailFrame = new QFrame;
    treeDetailFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
    treeDetailFrame->setLineWidth(0);
    treeDetailFrame->setContentsMargins(0, 0, 0, 0);
    treeDetailFrame->setLayout(tabLayout_);

    // Layout
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(treeWidget_);
    splitter->addWidget(treeDetailFrame);

    int w = width() / 4;
    splitter->setSizes(QList<int>({w, w * 3}));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(1, 1, 1, 1);

    setLayout(mainLayout);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(const QSet<Editor::Type> &)));
}

void ProjectNavigatorTree::addItem(ProjectNavigatorItem *widget)
{
    LOG_DEBUG_LOCAL(widget->text());

    block();

    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_);

    item->setText(COLUMN_ID, QString::number(tabList_.size()));
    if (widget->hasColorSource())
    {
        item->setCheckState(COLUMN_COLOR, Qt::Unchecked);
    }
    if (widget->hasFilter())
    {
        item->setCheckState(COLUMN_FILTER, Qt::Unchecked);
    }
    item->setIcon(COLUMN_LABEL, widget->icon());
    item->setText(COLUMN_LABEL, widget->text());

    // Register new tab
    tabList_.push_back(widget);

    if (tabList_.size() == 1)
    {
        widget->setVisible(true);
    }
    else
    {
        widget->setVisible(false);
    }

    tabLayout_->addWidget(widget);

    unblock();
}

void ProjectNavigatorTree::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (item == nullptr || column < 0)
    {
        return;
    }

    if (column == COLUMN_COLOR)
    {
        size_t idx = index(item);
        if (tabList_[idx]->hasColorSource())
        {
            SettingsView::ColorSource csrc = tabList_[idx]->colorSource();
            bool checked = (item->checkState(COLUMN_COLOR) == Qt::Checked);
            settings_.setColorSourceEnabled(csrc, checked);
            applySettingsOut();
        }
    }
    else if (column == COLUMN_FILTER)
    {
        size_t idx = index(item);
        if (tabList_[idx]->hasFilter())
        {
            bool checked = (item->checkState(COLUMN_FILTER) == Qt::Checked);
            tabList_[idx]->setFilterEnabled(checked);
        }
    }
}

void ProjectNavigatorTree::slotItemClicked(QTreeWidgetItem *item, int column)
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

size_t ProjectNavigatorTree::index(const QTreeWidgetItem *item)
{
    return item->text(COLUMN_ID).toULong();
}

void ProjectNavigatorTree::block()
{
    disconnect(treeWidget_, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 0, 0);
    disconnect(treeWidget_, SIGNAL(itemClicked(QTreeWidgetItem *, int)), 0, 0);
    (void)blockSignals(true);
}

void ProjectNavigatorTree::unblock()
{
    (void)blockSignals(false);
    connect(treeWidget_,
            SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    connect(treeWidget_,
            SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(slotItemClicked(QTreeWidgetItem *, int)));
}

void ProjectNavigatorTree::setTabVisible(size_t index)
{
    LOG_DEBUG_LOCAL("");

    for (size_t i = 0; i < tabList_.size(); i++)
    {
        if (i != index)
        {
            LOG_DEBUG_LOCAL("hide <" << i << ">");
            tabList_[i]->setVisible(false);
        }
    }

    if (index < tabList_.size())
    {
        LOG_DEBUG_LOCAL("show <" << index << ">");
        tabList_[index]->setVisible(true);
    }
}

void ProjectNavigatorTree::slotUpdate(const QSet<Editor::Type> &target)
{
    if (!target.empty() && !target.contains(Editor::TYPE_SETTINGS))
    {
        return;
    }

    applySettingsIn(mainWindow_->editor().settings().view());
}

void ProjectNavigatorTree::applySettingsIn(const SettingsView &settings)
{
    block();

    settings_ = settings;

    QTreeWidgetItemIterator it(treeWidget_);

    while (*it)
    {
        size_t idx = index(*it);

        if (tabList_[idx]->hasColorSource())
        {
            SettingsView::ColorSource csrc = tabList_[idx]->colorSource();
            if (settings_.isColorSourceEnabled(csrc))
            {
                (*it)->setCheckState(COLUMN_COLOR, Qt::Checked);
            }
            else
            {
                (*it)->setCheckState(COLUMN_COLOR, Qt::Unchecked);
            }
        }

        if (tabList_[idx]->hasFilter())
        {
            if (tabList_[idx]->isFilterEnabled())
            {
                (*it)->setCheckState(COLUMN_FILTER, Qt::Checked);
            }
            else
            {
                (*it)->setCheckState(COLUMN_FILTER, Qt::Unchecked);
            }
        }

        ++it;
    }

    unblock();
}

void ProjectNavigatorTree::applySettingsOut()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setSettingsView(settings_);
    mainWindow_->updateModifiers();
}
