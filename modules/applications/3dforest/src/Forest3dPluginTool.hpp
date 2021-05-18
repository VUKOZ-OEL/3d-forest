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

/**
    @file Forest3dPluginTool.hpp
*/

#ifndef FOREST_3D_PLUGIN_TOOL_HPP
#define FOREST_3D_PLUGIN_TOOL_HPP

#include <QString>
#include <QtPlugin>

class Forest3dEditor;
class QWidget;

/** Forest 3d Plugin Tool Interface. */
class Forest3dPluginTool
{
public:
    virtual ~Forest3dPluginTool() = default;
    virtual void show(QWidget *parent, Forest3dEditor *editor) = 0;
    virtual QString name() const = 0; // Unique menu name for each plugin
};

#define Forest3dPluginTool_iid "vukoz.3dforest.qt.Forest3dPluginTool/1.0"
Q_DECLARE_INTERFACE(Forest3dPluginTool, Forest3dPluginTool_iid)

#endif /* FOREST_3D_PLUGIN_TOOL_HPP */
