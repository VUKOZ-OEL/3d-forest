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

/** @file FilterTreesWindow.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <FilterTreesWidget.hpp>
#include <FilterTreesWindow.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterTreesWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/FilterTreesResources/", name))

FilterTreesWindow::FilterTreesWindow(Application *app) : DockWidget(app)
{
    // Widget.
    widget_ = new FilterTreesWidget(app);

    // Dock.
    setWidget(widget_);
    setWindowTitle(tr("Filter Trees"));
    setWindowIcon(ICON("tree"));
    setAllowedAreas(Ui::LeftDockWidgetArea | Ui::RightDockWidgetArea);
    app->addDockWidget(Ui::RightDockWidgetArea, this);
}

void FilterTreesWindow::showEvent(ShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    // widget_->setFilterEnabled(true);
    DockWidget::showEvent(event);
}

void FilterTreesWindow::hideEvent(HideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    DockWidget::hideEvent(event);
}

void FilterTreesWindow::closeEvent(CloseEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Close event.");
    // widget_->setFilterEnabled(false);
    event->accept();
}
