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

/** @file QtMainWindow.hpp */

#ifndef QT_MAIN_WINDOW_HPP
#define QT_MAIN_WINDOW_HPP

// Include std.
#include <functional>

// Include Qt.
#include <QMainWindow>
class QCloseEvent;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtMainWindow. */
class EXPORT_UI_QT QtMainWindow : public QMainWindow
{
public:
    using QMainWindow::QMainWindow;

    std::function<bool()> canClose;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#include <WarningsEnable.hpp>

#endif /* QT_MAIN_WINDOW_HPP */
