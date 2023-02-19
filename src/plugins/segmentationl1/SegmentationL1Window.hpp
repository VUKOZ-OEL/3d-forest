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

/** @file SegmentationL1Window.hpp */

#ifndef SEGMENTATION_L1_WINDOW_HPP
#define SEGMENTATION_L1_WINDOW_HPP

#include <AlgorithmWidgetInterface.hpp>
#include <SegmentationL1.hpp>
#include <SegmentationL1Parameters.hpp>
class MainWindow;
class SliderWidget;
class RangeSliderWidget;

/** Segmentation L1 Window. */
class SegmentationL1Window : public AlgorithmWidgetInterface
{
    Q_OBJECT

public:
    SegmentationL1Window(MainWindow *mainWindow);
    virtual ~SegmentationL1Window();

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

    SliderWidget *voxelSizeInput_;
    RangeSliderWidget *sampleDescriptorInput_;
    SliderWidget *numberOfSamplesInput_;
    RangeSliderWidget *neighborhoodRadiusInput_;
    SliderWidget *numberOfIterationsInput_;

    SegmentationL1 segmentationL1_;
    SegmentationL1Parameters parameters_;
};

#endif /* SEGMENTATION_L1_WINDOW_HPP */
