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

/** @file TreeTableWindow.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <TreeTableWidget.hpp>
#include <TreeTableWindow.hpp>

// Include Qt.
#include <QCloseEvent>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeTableWindow"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/TreeTableResources/", name))

TreeTableWindow::TreeTableWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      widget_(nullptr)
{
    LOG_DEBUG(<< "Create.");

    // Widget.
    widget_ = new TreeTableWidget(mainWindow);

    // Main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget_);

    // Dialog.
    setLayout(mainLayout);
    setWindowTitle(tr("Tree Table"));
    setWindowIcon(ICON("tree-table").icon());
    setModal(false);
}

TreeTableWindow::~TreeTableWindow()
{
    LOG_DEBUG(<< "Destroy.");
}

void TreeTableWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Close event.");

    widget_->closeWidget();

    event->accept();
}
