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

/** @file GuiPluginInterface.hpp */

#ifndef GUI_PLUGIN_INTERFACE_HPP
#define GUI_PLUGIN_INTERFACE_HPP

#include <QtPlugin>

#if QT_VERSION_MAJOR == 5
// Fix for qt5 which has two QAction classes
#include <QtWidgets/QAction>
#else
#include <QAction>
#endif

class GuiWindowMain;

/** Gui Plugin Interface. */
class GuiPluginInterface
{
public:
    virtual ~GuiPluginInterface() = default;
    virtual void initialize(GuiWindowMain *window) = 0;
};

#define GuiPluginInterface_iid "vukoz.3dforest.qt.GuiPluginInterface/1.0"
Q_DECLARE_INTERFACE(GuiPluginInterface, GuiPluginInterface_iid)

#endif /* GUI_PLUGIN_INTERFACE_HPP */
