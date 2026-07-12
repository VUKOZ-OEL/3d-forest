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

/** @file DockWidget.hpp */

#ifndef DOCK_WIDGET_HPP
#define DOCK_WIDGET_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <ThemeIcon.hpp>
#include <PaintEvent.hpp>
#include <MouseEvent.hpp>
#include <HideEvent.hpp>
#include <ShowEvent.hpp>
#include <CloseEvent.hpp>
#include <Ui.hpp>
class Application;
class Widget;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Dock Widget. */
class EXPORT_UI_COMMON DockWidget
{
public:
    DockWidget();
    DockWidget(Application *app);
    virtual ~DockWidget();

    void setWindowTitle(const std::string &str);
    void setWindowIcon(const ThemeIcon &icon);

    void setAllowedAreas(int areas);
    void setArea(int area);

    void setWidget(Widget *widget);
    Widget *widget() { return widget_; }

    void setFixedHeight(int h);

    void show() {};
    void raise() {};
    void activateWindow() {};

    virtual void paintEvent(PaintEvent *event);
    virtual void mousePressEvent(MouseEvent *event);
    virtual void hideEvent(HideEvent *event);
    virtual void showEvent(ShowEvent *event);
    virtual void closeEvent(CloseEvent *event);

private:
    Widget *widget_;
};

#include <WarningsEnable.hpp>

#endif /* DOCK_WIDGET_HPP */
