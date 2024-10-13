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
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <TreeTableExportCsv.hpp>
#include <TreeTableExportDialog.hpp>
#include <TreeTableWidget.hpp>
#include <Util.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeTableWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/treetable/", name))

TreeTableWidget::TreeTableWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    tableWidget_ = new QTableWidget();

    tableWidget_->setRowCount(0);
    tableWidget_->setColumnCount(2);

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

    // Layout.
    QVBoxLayout *tableLayout = new QVBoxLayout;
    tableLayout->addWidget(tableWidget_);
    tableLayout->addStretch();

    // Buttons.
    exportButton_ = new QPushButton(tr("Export"));
    exportButton_->setIcon(THEME_ICON("export_file"));
    exportButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(exportButton_, SIGNAL(clicked()), this, SLOT(slotExport()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(exportButton_);

    // Main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(tableLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget.
    setLayout(mainLayout);

    // Data.
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

        setSegments(mainWindow_->editor().segments());
    }
}

void TreeTableWidget::setSegments(const Segments &segments)
{
    LOG_DEBUG(<< "Set segments n <" << segments.size() << ">.");

    block();

    segments_ = segments;

    tableWidget_->clear();

    tableWidget_->setRowCount(static_cast<int>(segments_.size()));
    tableWidget_->setColumnCount(COLUMN_LAST);
    tableWidget_->setHorizontalHeaderLabels({"ID",
                                             "Label",
                                             "X [m]",
                                             "Y [m]",
                                             "Z [m]",
                                             "Height [m]",
                                             "DBH [m]",
                                             "Status"});

    // Content.
    for (size_t i = 0; i < segments_.size(); i++)
    {
        setRow(i);
    }

    // Resize Columns to the minimum space.
    // for (int i = 0; i < COLUMN_LAST; i++)
    // {
    //     tableWidget_->resizeColumnToContents(i);
    // }

    // Sort Content.
    tableWidget_->setSortingEnabled(true);
    tableWidget_->sortItems(COLUMN_ID, Qt::AscendingOrder);

    unblock();
}

void TreeTableWidget::setRow(size_t i)
{
    double ppm = mainWindow_->editor().settings().units().pointsPerMeter()[0];
    int row = static_cast<int>(i);

    const Segment &segment = segments_[i];
    const TreeAttributes &treeAttributes = segment.treeAttributes;

    setCell(row, COLUMN_ID, segment.id);
    setCell(row, COLUMN_LABEL, segment.label);
    setCell(row, COLUMN_X, treeAttributes.position[0] / ppm);
    setCell(row, COLUMN_Y, treeAttributes.position[1] / ppm);
    setCell(row, COLUMN_Z, treeAttributes.position[2] / ppm);
    setCell(row, COLUMN_HEIGHT, treeAttributes.height / ppm);
    setCell(row, COLUMN_STATUS, toString(treeAttributes.status));
    setCell(row, COLUMN_DBH, treeAttributes.dbh / ppm);
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

    tableWidget_->setItem(row, col, item);
}

void TreeTableWidget::block()
{
    (void)blockSignals(true);
}

void TreeTableWidget::unblock()
{
    (void)blockSignals(false);
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

            for (size_t i = 0; i < segments_.size(); i++)
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
