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
#include <TreeTableExportCsv.hpp>
#include <TreeTableExportDialog.hpp>
#include <TreeTableWidget.hpp>
#include <Util.hpp>

// Include Qt.
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
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

    // Widgets.
    tableWidget_ = new QTableWidget();

    tableWidget_->setRowCount(0);
    tableWidget_->setColumnCount(2);

    tableWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    tableWidget_->setStyleSheet("QHeaderView::section {"
                                "background-color: lightblue;"
                                "color: black;"
                                "padding: 5px;"
                                "}");

    // Enable alternating row colors
    tableWidget_->setAlternatingRowColors(true);

    QPalette palette = tableWidget_->palette();
    palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    palette.setColor(QPalette::Base, Qt::white);
    tableWidget_->setPalette(palette);

    // Buttons.
    showOnlyVisibleTreesButton_ = new QPushButton(tr("Show visible"));
    showOnlyVisibleTreesButton_->setSizePolicy(QSizePolicy::Minimum,
                                               QSizePolicy::Minimum);
    connect(showOnlyVisibleTreesButton_,
            SIGNAL(clicked()),
            this,
            SLOT(showOnlyVisibleTrees()));

    showAllTreesButton_ = new QPushButton(tr("Show all"));
    showAllTreesButton_->setSizePolicy(QSizePolicy::Minimum,
                                       QSizePolicy::Minimum);
    connect(showAllTreesButton_, SIGNAL(clicked()), this, SLOT(showAllTrees()));

    exportButton_ = new QPushButton(tr("Export"));
    exportButton_->setIcon(THEME_ICON("export-file"));
    exportButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(exportButton_, SIGNAL(clicked()), this, SLOT(slotExport()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(showOnlyVisibleTreesButton_);
    buttonsLayout->addWidget(showAllTreesButton_);
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

        setSegments(mainWindow_->editor().segments(),
                    mainWindow_->editor().segmentsFilter());
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
                                             "X [m]",
                                             "Y [m]",
                                             "Z [m]",
                                             "Height [m]",
                                             "DBH [m]",
                                             "Area [m^2]",
                                             "Status"});

    // Content.
    if (visibleTreesIdList_.size() > 0)
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

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    // size_t id = segments_.id(index);
    LOG_DEBUG(<< "Set id <" << segment.id << "> label <" << segment.label
              << ">.");

    setCell(row, COLUMN_ID, segment.id);
    setCell(row, COLUMN_LABEL, segment.label);
    setCell(row, COLUMN_X, treeAttributes.position[0] / ppm);
    setCell(row, COLUMN_Y, treeAttributes.position[1] / ppm);
    setCell(row, COLUMN_Z, treeAttributes.position[2] / ppm);
    setCell(row, COLUMN_HEIGHT, treeAttributes.height / ppm);
    setCell(row, COLUMN_STATUS, treeAttributes.isValid() ? "Valid" : "Invalid");
    setCell(row, COLUMN_DBH, treeAttributes.dbh / ppm);
    setCell(row, COLUMN_AREA, treeAttributes.area / (ppm * ppm));
}

void TreeTableWidget::setCell(int row, int col, bool value, bool userCheckable)
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
        setCell(row, col, value ? "Yes" : "No");
    }
}

void TreeTableWidget::setCell(int row, int col, size_t value)
{
    setCell(row, col, toString(value));
}

void TreeTableWidget::setCell(int row, int col, double value)
{
    setCell(row, col, toString(value, 3));
}

void TreeTableWidget::setCell(int row, int col, const std::string &value)
{
    QString text(QString::fromStdString(value));
    QTableWidgetItem *item = new QTableWidgetItem(text);
    /*
        if (col == COLUMN_ID)
        {
            // Color legend.
            size_t index = static_cast<size_t>(row);
            const Vector3<double> &rgb = segments_[index].color;

            QColor color;
            color.setRedF(static_cast<float>(rgb[0]));
            color.setGreenF(static_cast<float>(rgb[1]));
            color.setBlueF(static_cast<float>(rgb[2]));

            QBrush brush(color, Qt::SolidPattern);
            item->setBackground(brush);
        }
    */
    tableWidget_->setItem(row, col, item);
}

void TreeTableWidget::block()
{
    disconnect(tableWidget_, SIGNAL(cellClicked(int, int)), 0, 0);

    (void)blockSignals(true);
}

void TreeTableWidget::unblock()
{
    (void)blockSignals(false);

    connect(tableWidget_,
            SIGNAL(cellClicked(int, int)),
            this,
            SLOT(onCellClicked(int, int)));
}

void TreeTableWidget::showOnlyVisibleTrees()
{
    FindVisibleObjects::run(visibleTreesIdList_, mainWindow_);
    setTable();
}

void TreeTableWidget::showAllTrees()
{
    visibleTreesIdList_.clear();
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

            for (size_t i = 1; i < segments_.size(); i++)
            {
                writer->write(segments_[i]);
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

void TreeTableWidget::onCellClicked(int row, int column)
{
    LOG_DEBUG(<< "Start table clicked, row <" << row << "> column <" << column
              << ">.");

    QTableWidgetItem *itemId = tableWidget_->item(row, COLUMN_ID);
    if (!itemId)
    {
        LOG_ERROR(<< "Failed to get table item ID.");
    }

    QString textId = itemId->text();
    LOG_DEBUG(<< "Item text <" << textId.toStdString() << ">.");

    size_t id = textId.toULong();
    size_t index = segments_.index(id);

    LOG_DEBUG(<< "Set segment ID <" << id << "> selected.");

    for (size_t i = 0; i < segments_.size(); i++)
    {
        segments_[i].selected = false;
    }

    segments_[index].selected = true;

    if (updatesEnabled_)
    {
        dataChanged();
    }

    LOG_DEBUG(<< "Finished table clicked.");
}
