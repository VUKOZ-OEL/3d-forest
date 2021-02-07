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
    @file Forest3dPluginToolExample.hpp
*/

#ifndef FOREST_3D_PLUGIN_TOOL_EXAMPLE_HPP
#define FOREST_3D_PLUGIN_TOOL_EXAMPLE_HPP

#include <Editor.hpp>
#include <Forest3dPluginTool.hpp>
#include <QObject>
#include <QtPlugin>

/** Forest 3d Plugin Tool Example. */
class Forest3dPluginToolExample : public QObject, public Forest3dPluginTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Forest3dPluginTool_iid)
    Q_INTERFACES(Forest3dPluginTool)

public:
    Forest3dPluginToolExample();
    virtual ~Forest3dPluginToolExample();

    virtual void compute(Forest3dWindow *parent, Editor &editor);
    virtual QString name() const;
};

#endif /* FOREST_3D_PLUGIN_TOOL_EXAMPLE_HPP */
