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

/** @file FilterManagementStatusWindow.hpp */

#ifndef FILTER_MANAGEMENT_STATUS_WINDOW_HPP
#define FILTER_MANAGEMENT_STATUS_WINDOW_HPP

// Include 3D Forest.
class MainWindow;
class FilterManagementStatusWidget;

// Include Qt.
#include <QDockWidget>
class QCloseEvent;

/** Filter Management Status Window. */
class FilterManagementStatusWindow : public QDockWidget
{
    Q_OBJECT

public:
    FilterManagementStatusWindow(MainWindow *mainWindow);

protected:
    FilterManagementStatusWidget *widget_;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif /* FILTER_MANAGEMENT_STATUS_WINDOW_HPP */
