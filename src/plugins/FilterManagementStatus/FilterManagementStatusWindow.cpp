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

/** @file FilterManagementStatusWindow.cpp */

// Include 3D Forest.
#include <FilterManagementStatusWidget.hpp>
#include <FilterManagementStatusWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCloseEvent>

// Include local.
#define LOG_MODULE_NAME "FilterManagementStatusWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterManagementStatusResources/", name))

FilterManagementStatusWindow::FilterManagementStatusWindow(
    MainWindow *mainWindow)
    : QDockWidget(mainWindow)
{
    LOG_DEBUG(<< "Start creating management status window.");

    // Widget.
    widget_ = new FilterManagementStatusWidget(mainWindow);

    // Dock.
    setWidget(widget_);
    setWindowTitle(tr("Filter ManagementStatus"));
    setWindowIcon(ICON("management-status-filter").icon());
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);

    LOG_DEBUG(<< "Finished creating management status window.");
}

void FilterManagementStatusWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    // widget_->setFilterEnabled(true);
    QWidget::showEvent(event);
}

void FilterManagementStatusWindow::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    QWidget::hideEvent(event);
}

void FilterManagementStatusWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Close event.");
    // widget_->setFilterEnabled(false);
    event->accept();
}
