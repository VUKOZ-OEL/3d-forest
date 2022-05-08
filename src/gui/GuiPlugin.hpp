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

/** @file PluginTool.hpp */

#ifndef PLUGIN_TOOL_HPP
#define PLUGIN_TOOL_HPP

#include <QPixmap>
#include <QString>
#include <QtPlugin>

class Editor;
class QAction;
class QMainWindow;

/** Plugin Tool Interface. */
class PluginTool
{
public:
    virtual ~PluginTool() = default;
    virtual void initialize(QMainWindow *parent, Editor *editor) = 0;
    virtual void show(QMainWindow *parent) = 0;
    virtual QAction *toggleViewAction() const = 0;
    virtual QString windowTitle() const = 0; /**< Unique */
    virtual QString buttonText() const = 0;
    virtual QString toolTip() const = 0;
    virtual QPixmap icon() const = 0;
};

#define PluginTool_iid "vukoz.3dforest.qt.PluginTool/1.0"
Q_DECLARE_INTERFACE(PluginTool, PluginTool_iid)

#endif /* PLUGIN_TOOL_HPP */
