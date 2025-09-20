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

/** @file FilterDescriptorWindow.cpp */

// Include 3D Forest.
#include <FilterDescriptorWidget.hpp>
#include <FilterDescriptorWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCloseEvent>

// Include local.
#define LOG_MODULE_NAME "FilterDescriptorWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterDescriptorResources/", name))

FilterDescriptorWindow::FilterDescriptorWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow)
{
    LOG_DEBUG(<< "Start creating descriptor filter window.");

    // Widget.
    widget_ = new FilterDescriptorWidget(mainWindow);

    // Dock.
    setWidget(widget_);
    setFixedHeight(widget()->sizeHint().height());
    setWindowTitle(tr("Filter Descriptor"));
    setWindowIcon(ICON("descriptor-filter").icon());
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);

    LOG_DEBUG(<< "Finished creating descriptor filter window.");
}

void FilterDescriptorWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    // widget_->setFilterEnabled(true);
    QWidget::showEvent(event);
}

void FilterDescriptorWindow::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    QWidget::hideEvent(event);
}

void FilterDescriptorWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Close event.");
    // widget_->setFilterEnabled(false);
    event->accept();
}
