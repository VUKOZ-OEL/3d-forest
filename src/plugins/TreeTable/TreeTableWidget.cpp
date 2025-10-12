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

/** @file TreeTableWidget.cpp */

// Include 3D Forest.
#include <FindVisibleObjects.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <TreeTableAction.hpp>
#include <TreeTableExportCsv.hpp>
#include <TreeTableExportDialog.hpp>
#include <TreeTableSetManagementStatus.hpp>
#include <TreeTableSetSpecies.hpp>
#include <TreeTableWidget.hpp>
#include <Util.hpp>

// Include Qt.
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeTableWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/TreeTableResources/", name))

TreeTableWidget::TreeTableWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Table widget.
    tableWidget_ = new QTableWidget();

    tableWidget_->setRowCount(0);
    tableWidget_->setColumnCount(2);

    tableWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    tableWidget_->setStyleSheet("QHeaderView::section {"
                                "background-color: lightblue;"
                                "color: black;"
                                "padding: 5px;"
                                "}"
                                "QTableWidget::item:selected {"
                                "  background-color: #3399FF;"
                                "  color: white;"
                                "}");

    // Table: enable alternating row colors
    tableWidget_->setAlternatingRowColors(true);

    QPalette palette = tableWidget_->palette();
    palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    palette.setColor(QPalette::Base, Qt::white);
    tableWidget_->setPalette(palette);

    // Table: Context menu.
    tableWidget_->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tableWidget_,
            SIGNAL(customContextMenuRequested(QPoint)),
            this,
            SLOT(slotCustomContextMenuRequested(QPoint)));

    // Table: Selection.
    tableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Options.
    showOnlyVisibleTreesCheckBox_ = new QCheckBox;
    showOnlyVisibleTreesCheckBox_->setText(tr("Show only visible trees"));
    showOnlyVisibleTreesCheckBox_->setChecked(false);

    connect(showOnlyVisibleTreesCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotShowOnlyVisibleTreesChanged(int)));

    // Buttons.
    exportButton_ = new QPushButton(tr("Export"));
    exportButton_->setIcon(THEME_ICON("export-file").icon());
    exportButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(exportButton_, SIGNAL(clicked()), this, SLOT(slotExport()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(showOnlyVisibleTreesCheckBox_);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(exportButton_);

    // Main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tableWidget_, 1);
    mainLayout->addLayout(buttonsLayout, 0);

    // Widget.
    setLayout(mainLayout);

    // Data.
    updatesEnabled_ = true;
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());
}

void TreeTableWidget::slotUpdate(void *sender, const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_SEGMENT) ||
        target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input segments.");

        speciesList_ = mainWindow_->editor().speciesList();

        setSegments(mainWindow_->editor().segments(),
                    mainWindow_->editor().segmentsFilter());
    }

    if (target.contains(Editor::TYPE_FILTER))
    {
        showOnlyVisibleTreesUpdate();
    }
}

void TreeTableWidget::setSegments(const Segments &segments,
                                  const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set segments n <" << segments.size() << ">.");

    segments_ = segments;
    filter_ = filter;

    setTable();
}

void TreeTableWidget::dataChanged()
{
    LOG_DEBUG_UPDATE(<< "Start updating the changed segment data.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSegments(segments_);
    mainWindow_->editor().setSegmentsFilter(filter_);
    mainWindow_->updateData();

    LOG_DEBUG_UPDATE(<< "Finished updating the changed segment data.");
}

void TreeTableWidget::filterChanged()
{
    LOG_DEBUG_UPDATE(<< "Start updating the changed segment filter.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setSegmentsFilter(filter_);
    mainWindow_->updateFilter();

    LOG_DEBUG_UPDATE(<< "Finished updating the changed segment filter.");
}

void TreeTableWidget::setTable()
{
    block();

    tableWidget_->clear();
    tableWidget_->setSortingEnabled(false);

    tableWidget_->setColumnCount(COLUMN_LAST);
    tableWidget_->setHorizontalHeaderLabels({"ID",
                                             "Label",
                                             "Filter",
                                             "M.Status",
                                             "Species",
                                             "X [m]",
                                             "Y [m]",
                                             "Z [m]",
                                             "Height [m]",
                                             "DBH [m]",
                                             "Crown X [m]",
                                             "Crown Y [m]",
                                             "Crown Z [m]",
                                             "Area [m^2]",
                                             "Vol [m^3]",
                                             "Status"});

    // Content.
    if (showOnlyVisibleTreesCheckBox_->isChecked())
    {
        int nRows = static_cast<int>(visibleTreesIdList_.size());
        tableWidget_->setRowCount(nRows);
        LOG_DEBUG(<< "Set row count <" << nRows << ">.");

        int i = 0;
        for (const auto &id : visibleTreesIdList_)
        {
            size_t index = segments_.index(id, false);
            if (index != SIZE_MAX)
            {
                setRow(i, index);
                i++;
            }
        }
    }
    else
    {
        int nRows = static_cast<int>(segments_.size());
        tableWidget_->setRowCount(nRows);
        LOG_DEBUG(<< "Set row count <" << nRows << ">.");

        int i = 0;
        for (size_t index = 0; index < segments_.size(); index++)
        {
            setRow(i, index);
            i++;
        }
    }

    // Resize Columns to the minimum space.
    // for (int i = 0; i < COLUMN_LAST; i++)
    // {
    //     tableWidget_->resizeColumnToContents(i);
    // }

    tableWidget_->horizontalHeader()->setDefaultSectionSize(100);
    tableWidget_->setColumnWidth(COLUMN_ID, 20);

    // Sort Content.
    tableWidget_->setSortingEnabled(true);

    unblock();
}

void TreeTableWidget::setRow(int row, size_t index)
{
    LOG_DEBUG(<< "Set row <" << row << "> index <" << index << ">.");

    const Segment &segment = segments_[index];
    const TreeAttributes &treeAttributes = segment.treeAttributes;

    const auto &managementStatusList =
        mainWindow_->editor().managementStatusList();

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];
    double ppm2 = ppm * ppm;
    double ppm3 = ppm * ppm * ppm;

    // size_t id = segments_.id(index);
    LOG_DEBUG(<< "Set id <" << segment.id << "> label <" << segment.label
              << ">.");

    const Vector3<double> &treeColorRGB = segment.color;
    QColor treeColor;
    treeColor.setRedF(static_cast<float>(treeColorRGB[0]));
    treeColor.setGreenF(static_cast<float>(treeColorRGB[1]));
    treeColor.setBlueF(static_cast<float>(treeColorRGB[2]));

    const QueryFilterSet &filter = mainWindow_->editor().segmentsFilter();
    const std::unordered_set<size_t> &filterIdList = filter.filter();
    bool isInFilter = false;
    if (filterIdList.find(segment.id) != filterIdList.end())
    {
        isInFilter = true;
    }

    setCell(row, COLUMN_ID, segment.id, treeColor);
    setCell(row, COLUMN_LABEL, segment.label);
    setCell(row, COLUMN_FILTER, isInFilter ? "Yes" : "No");
    setCell(row,
            COLUMN_MANAGEMENT_STATUS,
            managementStatusList.labelById(segment.managementStatusId, false));
    setCell(row,
            COLUMN_SPECIES,
            speciesList_.labelById(segment.speciesId, false));
    setCell(row, COLUMN_X, treeAttributes.position[0] / ppm);
    setCell(row, COLUMN_Y, treeAttributes.position[1] / ppm);
    setCell(row, COLUMN_Z, treeAttributes.position[2] / ppm);
    setCell(row, COLUMN_HEIGHT, treeAttributes.height / ppm);
    setCell(row, COLUMN_STATUS, treeAttributes.isValid() ? "Valid" : "Invalid");
    setCell(row, COLUMN_DBH, treeAttributes.dbh / ppm);
    setCell(row, COLUMN_CROWN_X, treeAttributes.crownCenter[0] / ppm);
    setCell(row, COLUMN_CROWN_Y, treeAttributes.crownCenter[1] / ppm);
    setCell(row, COLUMN_CROWN_Z, treeAttributes.crownCenter[2] / ppm);
    setCell(row, COLUMN_AREA, treeAttributes.surfaceAreaProjection / ppm2);
    setCell(row, COLUMN_VOLUME, treeAttributes.volume / ppm3);
}

void TreeTableWidget::setCell(int row,
                              int col,
                              bool value,
                              bool userCheckable,
                              const QColor &color)
{
    if (userCheckable)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(value ? Qt::Checked : Qt::Unchecked);

        tableWidget_->setItem(row, col, item);
    }
    else
    {
        setCell(row, col, value ? "Yes" : "No", color);
    }
}

void TreeTableWidget::setCell(int row,
                              int col,
                              size_t value,
                              const QColor &color)
{
    setCell(row, col, toString(value), color);
}

void TreeTableWidget::setCell(int row,
                              int col,
                              double value,
                              const QColor &color)
{
    setCell(row, col, toString(value, 3), color);
}

void TreeTableWidget::setCell(int row,
                              int col,
                              const std::string &value,
                              const QColor &color)
{
    QString text(QString::fromStdString(value));
    QTableWidgetItem *item = new QTableWidgetItem(text);

    if (color.isValid())
    {
        QBrush brush(color, Qt::SolidPattern);
        item->setBackground(brush);
    }

    tableWidget_->setItem(row, col, item);
}

void TreeTableWidget::block()
{
    disconnect(tableWidget_, SIGNAL(customContextMenuRequested(QPoint)), 0, 0);

    (void)blockSignals(true);
}

void TreeTableWidget::unblock()
{
    (void)blockSignals(false);

    connect(tableWidget_,
            SIGNAL(customContextMenuRequested(QPoint)),
            this,
            SLOT(slotCustomContextMenuRequested(QPoint)));
}

void TreeTableWidget::showOnlyVisibleTreesUpdate()
{
    if (showOnlyVisibleTreesCheckBox_->isChecked())
    {
        FindVisibleObjects::run(visibleTreesIdList_, mainWindow_);
        setTable();
    }
    else if (!visibleTreesIdList_.empty())
    {
        visibleTreesIdList_.clear();
        setTable();
    }
}

void TreeTableWidget::slotShowOnlyVisibleTreesChanged(int index)
{
    (void)index;

    if (showOnlyVisibleTreesCheckBox_->isChecked())
    {
        FindVisibleObjects::run(visibleTreesIdList_, mainWindow_);
    }
    else
    {
        visibleTreesIdList_.clear();
    }

    setTable();
}

void TreeTableWidget::slotExport()
{
    LOG_DEBUG(<< "Start exporting tree table.");

    try
    {
        TreeTableExportDialog dialog(mainWindow_, fileName_);

        if (dialog.exec() == QDialog::Accepted)
        {
            std::shared_ptr<TreeTableExportInterface> writer = dialog.writer();

            writer->create(writer->properties().fileName());

            int rowCount = tableWidget_->rowCount();
            for (int row = 0; row < rowCount; ++row)
            {
                QTableWidgetItem *item = tableWidget_->item(row, COLUMN_ID);
                if (item)
                {
                    QString text = item->text();
                    size_t id = static_cast<size_t>(text.toULong());
                    size_t index = segments_.index(id, false);
                    if (index != SIZE_MAX)
                    {
                        writer->write(segments_[index], speciesList_);
                    }
                }
            }

            fileName_ = QString::fromStdString(writer->properties().fileName());
        }
    }
    catch (std::exception &e)
    {
        std::string msg("Export failed: ");
        msg += e.what();
        mainWindow_->showError(msg.c_str());
    }
    catch (...)
    {
        mainWindow_->showError("Export failed: Unknown error");
    }

    LOG_DEBUG(<< "Finished exporting tree table.");
}

void TreeTableWidget::slotCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = tableWidget_->indexAt(pos);
    if (!index.isValid())
    {
        return;
    }

    LOG_DEBUG(<< "Row <" << index.row() << "> column <" << index.column()
              << ">.");

    mainWindow_->suspendThreads();

    // Create and run the context menu.
    QMenu contextMenu(this);

    TreeTableSetManagementStatus managementStatusMenu(mainWindow_,
                                                      &contextMenu);
    TreeTableSetSpecies speciesMenu(mainWindow_, &contextMenu);
    QAction *showTreesAction = contextMenu.addAction("Show selected trees");
    QAction *hideTreesAction = contextMenu.addAction("Hide selected trees");

    QAction *selectedAction =
        contextMenu.exec(tableWidget_->viewport()->mapToGlobal(pos));

    // Selected rows to id list.
    QModelIndexList indexes = tableWidget_->selectionModel()->selectedIndexes();
    QSet<int> selectedRows;
    for (const QModelIndex &index : indexes)
    {
        selectedRows.insert(index.row());
    }

    std::unordered_set<size_t> idList;
    for (int row : selectedRows)
    {
        QTableWidgetItem *itemId = tableWidget_->item(row, COLUMN_ID);
        if (!itemId)
        {
            LOG_ERROR(<< "Failed to get table item ID at row <" << row << ">.");
            continue;
        }

        QString textId = itemId->text();
        idList.insert(static_cast<size_t>(textId.toULong()));
    }

    // Run selected action.
    managementStatusMenu.runAction(selectedAction, idList);
    speciesMenu.runAction(selectedAction, idList);

    if (selectedAction == showTreesAction)
    {
        TreeTableAction::showTrees(mainWindow_, idList);
        mainWindow_->update({Editor::TYPE_SEGMENT});
        setTable();
    }
    else if (selectedAction == hideTreesAction)
    {
        TreeTableAction::hideTrees(mainWindow_, idList);
        mainWindow_->update({Editor::TYPE_SEGMENT});
        setTable();
    }
}

void TreeTableWidget::closeWidget()
{
    if (showOnlyVisibleTreesCheckBox_->isChecked())
    {
        showOnlyVisibleTreesCheckBox_->setChecked(false);
        showOnlyVisibleTreesUpdate();
    }
}
