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

/** @file ComputeHeightMapPlugin.hpp */

#ifndef COMPUTE_HEIGHT_MAP_PLUGIN_HPP
#define COMPUTE_HEIGHT_MAP_PLUGIN_HPP

// Include 3D Forest.
#include <ComputeHeightMapModifier.hpp>
#include <ModifierInterface.hpp>
#include <PluginInterface.hpp>
class ComputeHeightMapWindow;

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestComputeHeightMapPlugin)
        #define EXPORT_COMPUTE_HEIGHT_MAP_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_COMPUTE_HEIGHT_MAP_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_COMPUTE_HEIGHT_MAP_PLUGIN
#endif

/** Compute Height Map Plugin.

    This class represents Compute Height Map plugin.
    This interface is visible to the application.

    This plugin provides on-the-fly interactive preview functionality.
    Plugins with interactive preview inherit ModifierInterface.
    Plugins with interactive preview feature have more complex design
    compared to solutions which use simple modal progress bar. The reason is
    that interactive preview creates concurrent access to memory from multiple
    threads.

    Compute Height Map plugin uses delayed lazy initialization of GUI widgets to
    save plugin loading time and memory.
*/
class EXPORT_COMPUTE_HEIGHT_MAP_PLUGIN ComputeHeightMapPlugin
    : public QObject,
      public PluginInterface,
      public ModifierInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    ComputeHeightMapPlugin();

    virtual void initialize(MainWindow *mainWindow);

    virtual bool modifierEnabled();
    virtual void applyModifier(Page *page);

public slots:
    void slotPlugin();

private:
    MainWindow *mainWindow_;

    /** First time use creates GUI. */
    ComputeHeightMapWindow *pluginWindow_;

    /** Must be created from the constructor.*/
    ComputeHeightMapModifier modifier_;
};

#endif /* COMPUTE_HEIGHT_MAP_PLUGIN_HPP */
