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

/** @file AlgorithmPluginInterface.hpp */

#ifndef ALGORITHM_PLUGIN_INTERFACE_HPP
#define ALGORITHM_PLUGIN_INTERFACE_HPP

#include <ExportGui.hpp>
class AlgorithmWidgetInterface;

#include <QtPlugin>

#if QT_VERSION_MAJOR == 5
    // Fix for qt5 which has two QAction classes
    #include <QtWidgets/QAction>
#else
    #include <QAction>
#endif

class MainWindow;

/** Algorithm Plugin Interface. */
class EXPORT_GUI AlgorithmPluginInterface
{
public:
    virtual ~AlgorithmPluginInterface() = default;
    virtual void initialize(MainWindow *mainWindow) = 0;
    virtual AlgorithmWidgetInterface *widget() = 0;
    virtual QString name() const = 0;
};

// clang-format off
#define AlgorithmPluginInterface_iid "vukoz.3dforest.qt.AlgorithmPluginInterface/1.0"
// clang-format on
Q_DECLARE_INTERFACE(AlgorithmPluginInterface, AlgorithmPluginInterface_iid)

#endif /* ALGORITHM_PLUGIN_INTERFACE_HPP */
