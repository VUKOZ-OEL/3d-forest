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

#include <IconTheme.hpp>
#include <MainWindow.hpp>
#include <SegmentationWindow.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#define ICON(name) (IconTheme(":/segmentation/", name))

SegmentationWindow::SegmentationWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Widgets
    distanceSpinBox_ = new QSpinBox;
    distanceSpinBox_->setRange(1, 100);
    distanceSpinBox_->setValue(1);
    distanceSpinBox_->setSingleStep(1);

    // Layout
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel(tr("Distance")), 0, 0);
    gridLayout->addWidget(distanceSpinBox_, 0, 1);

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(gridLayout);
    vboxLayout->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vboxLayout);
    widget_->setFixedHeight(150);
    setWidget(widget_);
    setWindowTitle(QObject::tr("Segmentation"));
    setFloating(true);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}

void SegmentationWindow::slotApply()
{
}
