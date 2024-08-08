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

/** @file ExplorerSegmentWidget.cpp */

// Include 3D Forest.
#include <ExplorerSegmentWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QTableWidget>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerSegmentWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ExplorerSegmentWidget::ExplorerSegmentWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Table.
    table_ = new QTableWidget();

    table_->setRowCount(0);
    table_->setColumnCount(2);

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(table_);

    setLayout(mainLayout);
}

void ExplorerSegmentWidget::setSegment(const Segment &segment)
{
    LOG_DEBUG(<< "Set segment id <" << segment.id << ">.");

    segment_ = segment;

    table_->clear();

    table_->setRowCount(6);
    table_->setColumnCount(2);
    table_->setHorizontalHeaderLabels({"Property", "Value"});

    double ppm = mainWindow_->editor().settings().units.pointsPerMeter()[0];

    setRow(0, "label", segment_.label);
    setRow(1, "x", segment_.position[0] / ppm, "m");
    setRow(2, "y", segment_.position[1] / ppm, "m");
    setRow(3, "z", segment_.position[2] / ppm, "m");
    setRow(4, "height", segment_.height / ppm, "m");
    setRow(5, "DBH", (segment_.radius * 2.0) / ppm, "m");
}

void ExplorerSegmentWidget::clear()
{
    LOG_DEBUG(<< "Clear data.");
    table_->clear();
}

void ExplorerSegmentWidget::setRow(int row,
                                   const std::string &key,
                                   const std::string &value)
{
    table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(key)));
    table_->setItem(row,
                    1,
                    new QTableWidgetItem(QString::fromStdString(value)));
}

void ExplorerSegmentWidget::setRow(int row,
                                   const std::string &key,
                                   double value,
                                   const std::string &comment)
{
    QString valueText;
    if (comment.empty())
    {
        valueText = QString::fromStdString(std::to_string(value));
    }
    else
    {
        valueText =
            QString::fromStdString(std::to_string(value) + " " + comment);
    }

    table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(key)));
    table_->setItem(row, 1, new QTableWidgetItem(valueText));
}
