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

/** @file HeightMapWindow.cpp */

#include <HeightMapWidget.hpp>
#include <HeightMapWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "HeightMapWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/heightmap/", name))

HeightMapWindow::HeightMapWindow(MainWindow *mainWindow,
                                 HeightMapModifier *modifier)
    : QDialog(mainWindow),
      widget_(nullptr)
{
    LOG_DEBUG(<< "Create.");

    // Widget
    widget_ = new HeightMapWidget(mainWindow, modifier);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget_);
    mainLayout->addStretch();

    // Dialog
    setLayout(mainLayout);
    setWindowTitle(tr("Height Map"));
    setWindowIcon(ICON("height_map"));
    setMaximumHeight(height());
    setModal(false);
}

void HeightMapWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Close event.");

    widget_->closeModifier();

    event->accept();
}
