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

/** @file SegmentationL1PluginWindow.cpp */

#include <MainWindow.hpp>
#include <RangeSliderWidget.hpp>
#include <SegmentationL1PluginConstants.hpp>
#include <SegmentationL1PluginWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QVBoxLayout>

#define LOG_MODULE_NAME "SegmentationL1PluginWindow"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/segmentationl1/", name))

SegmentationL1PluginWindow::SegmentationL1PluginWindow(MainWindow *mainWindow)
    : AlgorithmWidgetInterface(mainWindow,
                               ICON("forest"),
                               tr(SEGMENTATION_L1_PLUGIN_NAME)),
      mainWindow_(mainWindow),
      sampleDescriptorInput_(nullptr),
      numberOfSamplesInput_(nullptr),
      neighborhoodRadiusPcaInput_(nullptr),
      neighborhoodRadiusInput_(nullptr),
      numberOfIterationsInput_(nullptr),
      segmentationL1_(&mainWindow->editor()),
      parameters_()
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
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
                         tr("count"),
                         1,
                         1,
                         1000,
                         parameters_.numberOfSamples);

    SliderWidget::create(neighborhoodRadiusPcaInput_,
                         this,
                         nullptr,
                         SLOT(slotParametersChanged()),
                         tr("Neighborhood radius for PCA"),
                         tr("Neighborhood radius for PCA"),
                         tr("pt"),
                         1,
                         1,
                         10000,
                         parameters_.neighborhoodRadiusPca);

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
    settingsLayout->addWidget(sampleDescriptorInput_);
    settingsLayout->addWidget(numberOfSamplesInput_);
    settingsLayout->addWidget(neighborhoodRadiusPcaInput_);
    settingsLayout->addWidget(neighborhoodRadiusInput_);
    settingsLayout->addWidget(numberOfIterationsInput_);

    // Create widget layout.
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(settingsLayout);
    widgetLayout->addStretch();

    mainLayout_->addLayout(widgetLayout);
    setLayout(mainLayout_);
}

SegmentationL1PluginWindow::~SegmentationL1PluginWindow()
{
    LOG_DEBUG(<< "Destroy.");
}

bool SegmentationL1PluginWindow::applyParameters(bool autoStart)
{
    parameters_.set(sampleDescriptorInput_->minimumValue(),
                    sampleDescriptorInput_->maximumValue(),
                    numberOfSamplesInput_->value(),
                    neighborhoodRadiusPcaInput_->value(),
                    neighborhoodRadiusInput_->minimumValue(),
                    neighborhoodRadiusInput_->maximumValue(),
                    numberOfIterationsInput_->value());

    LOG_DEBUG(<< "Apply parameters <" << parameters_ << ">.");

    return segmentationL1_.applyParameters(parameters_, autoStart);
}

bool SegmentationL1PluginWindow::next()
{
    LOG_DEBUG(<< "Compute the next step.");
    return segmentationL1_.next();
}

void SegmentationL1PluginWindow::progress(size_t &nTasks,
                                          size_t &iTask,
                                          double &percent) const
{
    segmentationL1_.progress(nTasks, iTask, percent);
}

void SegmentationL1PluginWindow::updateData()
{
    LOG_DEBUG(<< "Update data.");
    mainWindow_->update({Editor::TYPE_LAYER});
}

void SegmentationL1PluginWindow::slotParametersChanged()
{
    LOG_DEBUG(<< "New value for some input parameter.");
    emit signalParametersChanged();
}
