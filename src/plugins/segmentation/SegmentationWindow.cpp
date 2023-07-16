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

/** @file SegmentationWindow.cpp */

#include <MainWindow.hpp>
#include <SegmentationWidget.hpp>
#include <SegmentationWindow.hpp>
#include <ThemeIcon.hpp>

#include <QVBoxLayout>

#define LOG_MODULE_NAME "SegmentationWindow"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/segmentation/", name))

SegmentationWindow::SegmentationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      widget_(nullptr)
{
    LOG_DEBUG(<< "Create.");

    // Widget
    widget_ = new SegmentationWidget(mainWindow);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget_);
    mainLayout->addStretch();

    // Dialog
    setLayout(mainLayout);
    setWindowTitle(tr("Segmentation"));
    setWindowIcon(ICON("forest"));
    setMaximumHeight(height());
    setModal(false);
}

SegmentationWindow::~SegmentationWindow()
{
    LOG_DEBUG(<< "Destroy.");
}
