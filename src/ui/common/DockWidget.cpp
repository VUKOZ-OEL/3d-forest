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

/** @file DockWidget.cpp */

// Include std.

// Include 3D Forest.
#include <DockWidget.hpp>
#include <Application.hpp>

// Include local.
#define LOG_MODULE_NAME "DockWidget"
#include <Log.hpp>

DockWidget::DockWidget()
{
}

DockWidget::DockWidget(Application *app)
{
}

DockWidget::~DockWidget()
{
}

void DockWidget::setWindowTitle(const std::string &str)
{
}

void DockWidget::setWindowIcon(const ThemeIcon &icon)
{
}

void DockWidget::setAllowedAreas(int areas)
{
}

void DockWidget::setArea(int area)
{
    // qt app_->addDockWidget(DockWidget::BottomDockWidgetArea, this);
}

void DockWidget::setWidget(Widget *widget)
{
}

void DockWidget::setFixedHeight(int h)
{
}

void DockWidget::paintEvent(PaintEvent *event)
{
}

void DockWidget::mousePressEvent(MouseEvent *event)
{
}

void DockWidget::hideEvent(HideEvent *event)
{
}

void DockWidget::showEvent(ShowEvent *event)
{
}

void DockWidget::closeEvent(CloseEvent *event)
{
}
