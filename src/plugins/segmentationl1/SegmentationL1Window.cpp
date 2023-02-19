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

/** @file SegmentationL1Window.cpp */

#include <MainWindow.hpp>
#include <RangeSliderWidget.hpp>
#include <SegmentationL1Constants.hpp>
#include <SegmentationL1Window.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QVBoxLayout>

#define LOG_MODULE_NAME "SegmentationL1Window"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/segmentationl1/", name))

SegmentationL1Window::SegmentationL1Window(MainWindow *mainWindow)
    : AlgorithmWidgetInterface(mainWindow,
                               ICON("forest"),
                               tr(SEGMENTATION_L1_NAME)),
      mainWindow_(mainWindow),
      voxelSizeInput_(nullptr),
      sampleDescriptorInput_(nullptr),
      numberOfSamplesInput_(nullptr),
      neighborhoodRadiusInput_(nullptr),
      numberOfIterationsInput_(nullptr),
      segmentationL1_(&mainWindow->editor()),
      parameters_()
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    SliderWidget::create(voxelSizeInput_,
                         this,
                         nullptr,
                         SLOT(slotParametersChanged()),
                         tr("Voxel Size"),
                         tr("Voxel Size"),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         parameters_.voxelSize);

    RangeSliderWidget::create(sampleDescriptorInput_,
                              this,
                              SLOT(slotParametersChanged()),
                              SLOT(slotParametersChanged()),
                              tr("Descriptor range of initial samples"),
                              tr("Descriptor range of initial samples"
                                 " to filter out leaves"),
                              tr("%"),
                              1,
                              0,
                              100,
                              parameters_.sampleDescriptorMinimum,
                              parameters_.sampleDescriptorMaximum);

    SliderWidget::create(numberOfSamplesInput_,
                         this,
                         nullptr,
                         SLOT(slotParametersChanged()),
                         tr("Number of initial samples"),
                         tr("Number of initial samples"),
                         tr("%"),
                         1,
                         1,
                         100,
                         parameters_.numberOfSamples);

    RangeSliderWidget::create(neighborhoodRadiusInput_,
                              this,
                              SLOT(slotParametersChanged()),
                              SLOT(slotParametersChanged()),
                              tr("Neighborhood radius range"),
                              tr("Neighborhood radius range"),
                              tr("pt"),
                              1,
                              1,
                              10000,
                              parameters_.neighborhoodRadiusMinimum,
                              parameters_.neighborhoodRadiusMaximum);

    SliderWidget::create(numberOfIterationsInput_,
                         this,
                         nullptr,
                         SLOT(slotParametersChanged()),
                         tr("Number of iterations"),
                         tr("Number of iterations"),
                         tr("cnt"),
                         1,
                         1,
                         100,
                         parameters_.numberOfIterations);

    // Create layout with parameters.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSizeInput_);
    settingsLayout->addWidget(sampleDescriptorInput_);
    settingsLayout->addWidget(numberOfSamplesInput_);
    settingsLayout->addWidget(neighborhoodRadiusInput_);
    settingsLayout->addWidget(numberOfIterationsInput_);

    // Create widget layout.
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(settingsLayout);
    widgetLayout->addStretch();

    mainLayout_->addLayout(widgetLayout);
    setLayout(mainLayout_);
}

SegmentationL1Window::~SegmentationL1Window()
{
    LOG_DEBUG(<< "Destroy.");
}

bool SegmentationL1Window::applyParameters()
{
    parameters_.set(voxelSizeInput_->value(),
                    numberOfSamplesInput_->value(),
                    sampleDescriptorInput_->minimumValue(),
                    sampleDescriptorInput_->maximumValue(),
                    neighborhoodRadiusInput_->minimumValue(),
                    neighborhoodRadiusInput_->maximumValue(),
                    numberOfIterationsInput_->value());

    LOG_DEBUG(<< "Apply parameters <" << parameters_ << ">.");

    return segmentationL1_.applyParameters(parameters_);
}

bool SegmentationL1Window::next()
{
    LOG_DEBUG(<< "Compute the next step.");
    return segmentationL1_.next();
}

void SegmentationL1Window::progress(size_t &nTasks,
                                    size_t &iTask,
                                    double &percent) const
{
    segmentationL1_.progress(nTasks, iTask, percent);
}

void SegmentationL1Window::updateData()
{
    LOG_DEBUG(<< "Update data.");
    mainWindow_->update({Editor::TYPE_LAYER});
}

void SegmentationL1Window::slotParametersChanged()
{
    LOG_DEBUG(<< "New value for some input parameter.");
    emit signalParametersChanged();
}
