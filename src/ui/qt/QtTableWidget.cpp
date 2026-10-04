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

/** @file QtTableWidget.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <QtTableWidget.hpp>

// Include Qt.
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPointer>
#include <QSignalBlocker>
#include <QVariant>

// Include local.
#define LOG_MODULE_NAME "QtTableWidget"
#include <Log.hpp>

namespace
{

QBrush toQtBrush(const Brush &brush)
{
    if (!brush.hasColor())
    {
        return QBrush();
    }

    return QBrush(
        QColor(brush.red(), brush.green(), brush.blue(), brush.alpha()));
}

void applyHeaderSettings(QHeaderView *target, const HeaderView &source)
{
    target->setVisible(source.isVisible());
    target->setStretchLastSection(source.stretchLastSection());

    if (source.defaultSectionSize() >= 0)
    {
        target->setDefaultSectionSize(source.defaultSectionSize());
    }
}

} // namespace

QtTableWidget::QtTableWidget(TableWidget *table, QWidget *parent)
    : QTableWidget(parent),
      table_(table)
{
    // Common TableWidget owns sorting and row order.
    QTableWidget::setSortingEnabled(false);

    const QPointer<QtTableWidget> guard(this);

    table_->tableReset.connect(
        [guard]
        {
            if (guard)
            {
                guard->rebuild();
            }
        });

    table_->cellUpdated.connect(
        [guard](int row, int column)
        {
            if (guard)
            {
                guard->updateCell(row, column);
            }
        });

    table_->headersUpdated.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateHeaders();
            }
        });

    table_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    table_->selectionUpdated.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSelection();
            }
        });

    table_->columnSizeRequested.connect(
        [guard](int column)
        {
            if (guard)
            {
                guard->updateColumnSize(column);
            }
        });

    table_->horizontalHeader()->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateHeaderSettings();
            }
        });

    table_->verticalHeader()->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateHeaderSettings();
            }
        });

    table_->destroying.connect(
        [guard]
        {
            if (guard)
            {
                delete guard.data();
            }
        });

    table_->setViewQueries(
        [guard](const Point &position)
        {
            if (!guard)
            {
                return ModelIndex();
            }

            const QModelIndex index = guard->QTableWidget::indexAt(
                QPoint(position.x(), position.y()));

            return index.isValid() ? ModelIndex(index.row(), index.column())
                                   : ModelIndex();
        },
        [guard](const Point &position)
        {
            if (!guard)
            {
                throw std::logic_error("Table viewport was destroyed.");
            }

            const QPoint global = guard->QTableWidget::viewport()->mapToGlobal(
                QPoint(position.x(), position.y()));

            return Point(global.x(), global.y());
        });

    QObject::connect(this,
                     &QTableWidget::cellClicked,
                     this,
                     [this](int row, int column)
                     {
                         if (auto *value = table_->item(row, column))
                         {
                             if (!table_->signalsBlocked())
                             {
                                 table_->itemClicked(value, column);
                             }
                         }
                     });

    QObject::connect(this,
                     &QTableWidget::cellChanged,
                     this,
                     [this](int row, int column) { readCell(row, column); });

    QObject::connect(selectionModel(),
                     &QItemSelectionModel::selectionChanged,
                     this,
                     [this](const QItemSelection &, const QItemSelection &)
                     { readSelection(); });

    QObject::connect(this,
                     &QWidget::customContextMenuRequested,
                     this,
                     [this](const QPoint &position)
                     {
                         if (!table_->signalsBlocked())
                         {
                             table_->customContextMenuRequested(
                                 Point(position.x(), position.y()));
                         }
                     });

    QObject::connect(QTableWidget::horizontalHeader(),
                     &QHeaderView::sortIndicatorChanged,
                     this,
                     [this](int column, Qt::SortOrder order)
                     {
                         if (table_->isSortingEnabled())
                         {
                             table_->sortItems(column,
                                               order == Qt::AscendingOrder
                                                   ? Ui::AscendingOrder
                                                   : Ui::DescendingOrder);
                         }
                     });

    updateSettings();
    rebuild();
}

QtTableWidget::~QtTableWidget()
{
}

void QtTableWidget::copyItem(QTableWidgetItem *target,
                             const TableWidgetItem &source)
{
    target->setText(QString::fromStdString(source.text()));
    target->setFlags(Qt::ItemFlags(source.flags()));
    target->setBackground(toQtBrush(source.background()));

    if (source.hasCheckState())
    {
        target->setCheckState(static_cast<Qt::CheckState>(source.checkState()));
    }
    else
    {
        target->setData(Qt::CheckStateRole, QVariant());
    }
}

void QtTableWidget::rebuild()
{
    const QSignalBlocker tableBlocker(this);
    const QSignalBlocker selectionBlocker(selectionModel());
    const QSignalBlocker headerBlocker(QTableWidget::horizontalHeader());

    const bool previouslyEnabled = updatesEnabled();
    setUpdatesEnabled(false);

    QTableWidget::clear();
    QTableWidget::setRowCount(table_->rowCount());
    QTableWidget::setColumnCount(table_->columnCount());

    updateHeaders();

    for (int row = 0; row < table_->rowCount(); ++row)
    {
        for (int column = 0; column < table_->columnCount(); ++column)
        {
            if (auto *source = table_->item(row, column))
            {
                auto *target = new QTableWidgetItem;
                copyItem(target, *source);
                QTableWidget::setItem(row, column, target);
            }
        }
    }

    updateSelection();

    for (const auto &entry : table_->columnWidths())
    {
        updateColumnSize(entry.first);
    }

    setUpdatesEnabled(previouslyEnabled);
}

void QtTableWidget::updateCell(int row, int column)
{
    const QSignalBlocker tableBlocker(this);
    const QSignalBlocker selectionBlocker(selectionModel());

    auto *source = table_->item(row, column);

    if (!source)
    {
        delete QTableWidget::takeItem(row, column);
        return;
    }

    auto *target = QTableWidget::item(row, column);

    if (!target)
    {
        target = new QTableWidgetItem;
        copyItem(target, *source);
        QTableWidget::setItem(row, column, target);
    }
    else
    {
        copyItem(target, *source);
    }
}

void QtTableWidget::updateHeaders()
{
    const QSignalBlocker blocker(this);

    for (int column = 0; column < table_->columnCount(); ++column)
    {
        const auto *source = table_->horizontalHeaderItem(column);

        if (!source)
        {
            delete QTableWidget::takeHorizontalHeaderItem(column);
            continue;
        }

        auto *target = QTableWidget::horizontalHeaderItem(column);

        if (!target)
        {
            target = new QTableWidgetItem;
            copyItem(target, *source);
            QTableWidget::setHorizontalHeaderItem(column, target);
        }
        else
        {
            copyItem(target, *source);
        }
    }
}

void QtTableWidget::updateSettings()
{
    const QSignalBlocker tableBlocker(this);
    const QSignalBlocker selectionBlocker(selectionModel());

    QTableWidget::setSelectionMode(
        static_cast<QAbstractItemView::SelectionMode>(table_->selectionMode()));

    QTableWidget::setSelectionBehavior(
        static_cast<QAbstractItemView::SelectionBehavior>(
            table_->selectionBehavior()));

    // QTableWidget::setAlternatingRowColors(table_->alternatingRowColors());
    QTableWidget::setAlternatingRowColors(false);

    QTableWidget::setContextMenuPolicy(
        static_cast<Qt::ContextMenuPolicy>(table_->contextMenuPolicy()));

    QHeaderView *header = QTableWidget::horizontalHeader();
    const QSignalBlocker headerBlocker(header);

    header->setSectionsClickable(table_->isSortingEnabled());
    header->setSortIndicatorShown(table_->isSortingEnabled());

    header->setSortIndicator(table_->sortColumn(),
                             table_->sortOrder() == Ui::AscendingOrder
                                 ? Qt::AscendingOrder
                                 : Qt::DescendingOrder);

    updateHeaderSettings();
    updateSelection();
}

void QtTableWidget::updateHeaderSettings()
{
    applyHeaderSettings(QTableWidget::horizontalHeader(),
                        *table_->horizontalHeader());

    applyHeaderSettings(QTableWidget::verticalHeader(),
                        *table_->verticalHeader());

    // Explicit column widths take priority over the default section size.
    for (const auto &entry : table_->columnWidths())
    {
        updateColumnSize(entry.first);
    }
}

void QtTableWidget::updateColumnSize(int column)
{
    const auto it = table_->columnWidths().find(column);

    if (it == table_->columnWidths().end())
    {
        return;
    }

    if (it->second < 0)
    {
        QTableWidget::resizeColumnToContents(column);
    }
    else
    {
        QTableWidget::setColumnWidth(column, it->second);
    }
}

void QtTableWidget::updateSelection()
{
    // Avoid clearing/reapplying a selection already made by Qt.
    TableWidget::Selection current;

    for (const QModelIndex &index : selectionModel()->selectedIndexes())
    {
        current.emplace(index.row(), index.column());
    }

    if (current == table_->selectedCells())
    {
        return;
    }

    const QSignalBlocker tableBlocker(this);
    const QSignalBlocker selectionBlocker(selectionModel());

    QItemSelection selection;

    for (const auto &cell : table_->selectedCells())
    {
        const QModelIndex index = model()->index(cell.first, cell.second);
        selection.select(index, index);
    }

    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void QtTableWidget::readSelection()
{
    TableWidget::Selection selection;

    for (const QModelIndex &index : selectionModel()->selectedIndexes())
    {
        selection.emplace(index.row(), index.column());
    }

    table_->setSelectedCells(selection, true);
}

void QtTableWidget::readCell(int row, int column)
{
    auto *native = QTableWidget::item(row, column);
    auto *common = table_->item(row, column);

    if (!native || !common)
    {
        return;
    }

    const std::string text = native->text().toStdString();

    const bool checkChanged =
        common->hasCheckState() && static_cast<int>(common->checkState()) !=
                                       static_cast<int>(native->checkState());

    if (text == common->text() && !checkChanged)
    {
        return;
    }

    // Snapshot both values before notifying the common table.
    // Its update may sort rows and rebuild the Qt items.
    TableWidgetItem edited(*common);
    edited.setText(text);

    if (common->hasCheckState())
    {
        edited.setCheckState(static_cast<Ui::CheckState>(native->checkState()));
    }

    table_->setItem(row, column, edited, true);
}
