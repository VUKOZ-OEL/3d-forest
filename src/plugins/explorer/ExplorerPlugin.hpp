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

/** @file ExplorerPlugin.hpp */

#ifndef EXPLORER_PLUGIN_HPP
#define EXPLORER_PLUGIN_HPP

// Include 3D Forest.
class MainWindow;
class ExplorerWindow;

// Include Qt.
#include <QObject>
class QDockWidget;

// Include local.
#include <ExportGui.hpp>

/** Explorer Plugin. */
class EXPORT_GUI ExplorerPlugin : public QObject
{
    Q_OBJECT

public:
    ExplorerPlugin();

    void initialize(MainWindow *mainWindow);
    QDockWidget *window() const;

public slots:
    void slotPlugin();

protected:
    MainWindow *mainWindow_;
    ExplorerWindow *projectWindow_;
};

#endif /* EXPLORER_PLUGIN_HPP */
