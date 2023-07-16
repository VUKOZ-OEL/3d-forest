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

/** @file SegmentationPlugin.hpp */

#ifndef SEGMENTATION_PLUGIN_HPP
#define SEGMENTATION_PLUGIN_HPP

#include <PluginInterface.hpp>
class SegmentationWindow;

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestSegmentationPlugin)
        #define EXPORT_SEGMENTATION_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_SEGMENTATION_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_SEGMENTATION_PLUGIN
#endif

/** Segmentation Plugin. */
class EXPORT_SEGMENTATION_PLUGIN SegmentationPlugin : public QObject,
                                                      public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    SegmentationPlugin();

    virtual void initialize(MainWindow *mainWindow);

public slots:
    void slotPlugin();

private:
    MainWindow *mainWindow_;
    SegmentationWindow *pluginWindow_;
};

#endif /* SEGMENTATION_PLUGIN_HPP */
