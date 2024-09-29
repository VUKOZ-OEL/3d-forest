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

/** @file ClassificationFilterWindow.cpp */

// Include 3D Forest.
#include <ClassificationFilterWidget.hpp>
#include <ClassificationFilterWindow.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCloseEvent>

// Include local.
#define LOG_MODULE_NAME "ClassificationFilterWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/classificationfilter/", name))

ClassificationFilterWindow::ClassificationFilterWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow)
{
    LOG_DEBUG(<< "Start creating classifications window.");

    // Widget.
    widget_ = new ClassificationFilterWidget(mainWindow);

    // Dock.
    setWidget(widget_);
    setWindowTitle(tr("Classification Filter"));
    setWindowIcon(ICON("classification_filter"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);

    LOG_DEBUG(<< "Finished creating classifications window.");
}

void ClassificationFilterWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    widget_->setFilterEnabled(true);
    QWidget::showEvent(event);
}

void ClassificationFilterWindow::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    QWidget::hideEvent(event);
}

void ClassificationFilterWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Close event.");
    widget_->setFilterEnabled(false);
    event->accept();
}
