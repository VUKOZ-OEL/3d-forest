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

/** @file FilterSpeciesWindow.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <FilterSpeciesWidget.hpp>
#include <FilterSpeciesWindow.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterSpeciesWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterSpeciesResources/", name))

FilterSpeciesWindow::FilterSpeciesWindow(Application *app) : DockWidget(app)
{
    LOG_DEBUG(<< "Start creating species window.");

    // Widget.
    widget_ = new FilterSpeciesWidget(app);

    // Dock.
    setWidget(widget_);
    setWindowTitle(tr("Filter Species"));
    setWindowIcon(ICON("species-filter"));
    setAllowedAreas(Ui::LeftDockWidgetArea | Ui::RightDockWidgetArea);
    app->addDockWidget(Ui::RightDockWidgetArea, this);

    LOG_DEBUG(<< "Finished creating species window.");
}

void FilterSpeciesWindow::showEvent(ShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    // widget_->setFilterEnabled(true);
    DockWidget::showEvent(event);
}

void FilterSpeciesWindow::hideEvent(HideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    DockWidget::hideEvent(event);
}

void FilterSpeciesWindow::closeEvent(CloseEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Close event.");
    // widget_->setFilterEnabled(false);
    event->accept();
}
