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

/** @file DensityPlugin.hpp */

#ifndef DENSITY_PLUGIN_HPP
#define DENSITY_PLUGIN_HPP

#include <AlgorithmPluginInterface.hpp>
#include <DensityConstants.hpp>
#include <DensityWindow.hpp>
#include <ExportDensity.hpp>

/** Density Plugin. */
class EXPORT_DENSITY_PLUGIN DensityPlugin : public QObject,
                                            public AlgorithmPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AlgorithmPluginInterface_iid)
    Q_INTERFACES(AlgorithmPluginInterface)

public:
    DensityPlugin();

    virtual void initialize(MainWindow *mainWindow);
    virtual AlgorithmWidgetInterface *widget() { return pluginWindow_; }
    virtual QString name() const { return DENSITY_NAME; }

private:
    MainWindow *mainWindow_;
    DensityWindow *pluginWindow_;
};

#endif /* DENSITY_PLUGIN_HPP */
