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

/** @file QtTreeWidget.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <QtTreeWidget.hpp>
#include <QtTreeWidgetItem.hpp>

// Include Qt.
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QPointer>
#include <QSignalBlocker>
#include <QStringList>
#include <QVariant>

// Include local.
#define LOG_MODULE_NAME "QtTreeWidget"
#include <Log.hpp>

QtTreeWidget::QtTreeWidget(TreeWidget *tree, QWidget *parent)
    : QTreeWidget(parent),
      tree_(tree)
{
    updateSettings();

    for (const auto &item : tree_->items())
    {
        insertItem(item.get());
    }

    updateSelection();
    resizeColumns();

    const QPointer<QtTreeWidget> guard(this);

    tree_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    tree_->clearing.connect(
        [guard]
        {
            if (guard)
            {
                const QSignalBlocker blocker(guard.data());
                guard->clear();
                guard->items_.clear();
            }
        });

    tree_->itemInserted.connect(
        [guard](TreeWidgetItem *item)
        {
            if (guard)
            {
                guard->insertItem(item);
            }
        });

    tree_->itemUpdated.connect(
        [guard](TreeWidgetItem *item, int)
        {
            if (guard)
            {
                guard->updateItem(item);
                guard->resizeColumns();
            }
        });

    tree_->selectionUpdated.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSelection();
            }
        });

    tree_->orderUpdated.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateOrder();
            }
        });

    tree_->resizeColumnRequested.connect(
        [guard](int column)
        {
            if (guard)
            {
                guard->resizeColumnToContents(column);
            }
        });

    QObject::connect(this,
                     &QTreeWidget::itemClicked,
                     this,
                     [this](QTreeWidgetItem *item, int column)
                     {
                         auto *qtItem = dynamic_cast<QtTreeWidgetItem *>(item);

                         if (qtItem && !tree_->signalsBlocked())
                         {
                             tree_->itemClicked(qtItem->commonItem(), column);
                         }
                     });

    QObject::connect(this,
                     &QTreeWidget::itemChanged,
                     this,
                     [this](QTreeWidgetItem *item, int column)
                     {
                         auto *qtItem = dynamic_cast<QtTreeWidgetItem *>(item);

                         if (!qtItem || column < 0)
                         {
                             return;
                         }

                         TreeWidgetItem *common = qtItem->commonItem();

                         const std::string text =
                             item->text(column).toStdString();
                         const bool checkable = common->isCheckable(column);
                         const int state =
                             static_cast<int>(item->checkState(column));

                         const bool changed =
                             common->text(column) != text ||
                             (checkable && common->checkState(column) != state);

                         if (!changed)
                         {
                             return;
                         }

                         common->setText(column, text);

                         if (checkable)
                         {
                             common->setCheckState(column, state);
                         }

                         // Notify the plugin once, after synchronizing all
                         // common data.
                         if (!tree_->signalsBlocked())
                         {
                             tree_->itemChanged(common, column);
                         }
                     });

    QObject::connect(
        this,
        &QTreeWidget::itemSelectionChanged,
        this,
        [this]
        {
            std::vector<TreeWidgetItem *> selected;

            for (QTreeWidgetItem *item : QTreeWidget::selectedItems())
            {
                if (auto *qtItem = dynamic_cast<QtTreeWidgetItem *>(item))
                {
                    selected.push_back(qtItem->commonItem());
                }
            }

            tree_->setSelectedItems(selected, true);
        });

    QObject::connect(header(),
                     &QHeaderView::sortIndicatorChanged,
                     this,
                     [this](int column, Qt::SortOrder order)
                     {
                         if (tree_->isSortingEnabled())
                         {
                             tree_->sortItems(column,
                                              order == Qt::AscendingOrder
                                                  ? Ui::AscendingOrder
                                                  : Ui::DescendingOrder);
                         }
                     });
}

QtTreeWidget::~QtTreeWidget() = default;

void QtTreeWidget::updateSettings()
{
    const QSignalBlocker blocker(this);
    const QSignalBlocker headerBlocker(header());

    setColumnCount(tree_->columnCount());

    QStringList labels;

    for (int column = 0; column < tree_->columnCount(); ++column)
    {
        const auto &headers = tree_->headerLabels();

        labels.append(column < static_cast<int>(headers.size())
                          ? QString::fromStdString(headers[column])
                          : QString());
    }

    setHeaderLabels(labels);

    setSelectionMode(
        static_cast<QAbstractItemView::SelectionMode>(tree_->selectionMode()));

    setSelectionBehavior(static_cast<QAbstractItemView::SelectionBehavior>(
        tree_->selectionBehavior()));

    header()->setSortIndicator(tree_->sortColumn(),
                               tree_->sortOrder() == Ui::AscendingOrder
                                   ? Qt::AscendingOrder
                                   : Qt::DescendingOrder);

    setSortingEnabled(tree_->isSortingEnabled());

    if (tree_->isSortingEnabled())
    {
        updateOrder();
    }

    updateSelection();
    resizeColumns();
}

void QtTreeWidget::updateOrder()
{
    const QSignalBlocker blocker(this);
    const QSignalBlocker headerBlocker(header());

    QTreeWidget::sortItems(tree_->sortColumn(),
                           tree_->sortOrder() == Ui::AscendingOrder
                               ? Qt::AscendingOrder
                               : Qt::DescendingOrder);
}

void QtTreeWidget::insertItem(TreeWidgetItem *item)
{
    const QSignalBlocker blocker(this);

    auto *qtItem = new QtTreeWidgetItem(item);
    items_.emplace(item, qtItem);

    // Populate before insertion so sorting sees the correct text.
    updateItem(item);

    if (item->parent())
    {
        items_.at(item->parent())->addChild(qtItem);
    }
    else
    {
        addTopLevelItem(qtItem);
    }

    for (const auto &child : item->children())
    {
        insertItem(child.get());
    }

    resizeColumns();
}

void QtTreeWidget::updateItem(TreeWidgetItem *item)
{
    const auto found = items_.find(item);

    if (found == items_.end())
    {
        return;
    }

    const QSignalBlocker blocker(this);
    QtTreeWidgetItem *qtItem = found->second;

    bool checkable = false;

    for (int column = 0; column < item->columnCount(); ++column)
    {
        qtItem->setText(column, QString::fromStdString(item->text(column)));

        if (item->isCheckable(column))
        {
            checkable = true;

            qtItem->setCheckState(
                column,
                static_cast<Qt::CheckState>(item->checkState(column)));
        }
        else
        {
            qtItem->setData(column, Qt::CheckStateRole, QVariant());
        }

        const Brush brush = item->background(column);

        if (brush.hasColor())
        {
            qtItem->setBackground(column,
                                  QBrush(QColor(brush.red(),
                                                brush.green(),
                                                brush.blue(),
                                                brush.alpha())));
        }
        else
        {
            qtItem->setBackground(column, QBrush());
        }
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (item->isEditable())
    {
        flags |= Qt::ItemIsEditable;
    }

    if (checkable)
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    qtItem->setFlags(flags);
}

void QtTreeWidget::updateSelection()
{
    const auto selected = tree_->selectedItems();
    const auto current = QTreeWidget::selectedItems();

    // Avoid replacing the selection that just came from Qt.
    const bool unchanged =
        selected.size() == static_cast<size_t>(current.size()) &&
        std::all_of(selected.begin(),
                    selected.end(),
                    [&](TreeWidgetItem *item)
                    {
                        const auto found = items_.find(item);

                        return found != items_.end() &&
                               current.contains(found->second);
                    });

    if (unchanged)
    {
        return;
    }

    const QSignalBlocker blocker(this);

    clearSelection();

    for (TreeWidgetItem *item : selected)
    {
        const auto found = items_.find(item);

        if (found != items_.end())
        {
            found->second->setSelected(true);
        }
    }
}

void QtTreeWidget::resizeColumns()
{
    for (int column : tree_->autoResizeColumns())
    {
        if (column < columnCount())
        {
            resizeColumnToContents(column);
        }
    }
}
