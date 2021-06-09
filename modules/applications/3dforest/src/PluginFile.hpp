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
    @file PluginFile.hpp
*/

#ifndef PLUGIN_FILE_HPP
#define PLUGIN_FILE_HPP

#include <QString>
#include <QtPlugin>

class QWidget;
class Editor;

/** Plugin File Interface. */
class PluginFile
{
public:
    virtual ~PluginFile() = default;
    virtual void read(QWidget *parent, Editor *editor) = 0;
    virtual void write(QWidget *parent, Editor *editor) = 0;
    virtual QString windowTitle() const = 0; /**< Unique */
};

#define PluginFile_iid "vukoz.3dforest.qt.PluginFile/1.0"
Q_DECLARE_INTERFACE(PluginFile, PluginFile_iid)

#endif /* PLUGIN_FILE_HPP */
