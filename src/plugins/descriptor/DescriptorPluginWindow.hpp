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

/** @file DescriptorPluginWindow.hpp */

#ifndef DESCRIPTOR_PLUGIN_WINDOW_HPP
#define DESCRIPTOR_PLUGIN_WINDOW_HPP

#include <AlgorithmWidgetInterface.hpp>
#include <DescriptorAlgorithm.hpp>
#include <DescriptorAlgorithmParameters.hpp>
class MainWindow;
class SliderWidget;

class QRadioButton;

/** Descriptor Plugin Window. */
class DescriptorPluginWindow : public AlgorithmWidgetInterface
{
    Q_OBJECT

public:
    DescriptorPluginWindow(MainWindow *mainWindow);
    virtual ~DescriptorPluginWindow();

    /** @name Algorithm interface. */
    /**@{*/
    virtual bool applyParameters();
    virtual bool next();
    virtual void progress(size_t &nTasks, size_t &iTask, double &percent) const;
    virtual void updateData();
    /**@}*/

protected slots:
    void slotParametersChanged();

private:
    MainWindow *mainWindow_;
    QRadioButton *methodPcaRadioButton_;
    QRadioButton *methodDensityRadioButton_;
    SliderWidget *neighborhoodRadiusInput_;
    DescriptorAlgorithmParameters parameters_;
    DescriptorAlgorithm densityAlgorithm_;
};

#endif /* DESCRIPTOR_PLUGIN_WINDOW_HPP */
