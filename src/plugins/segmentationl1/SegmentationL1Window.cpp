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
      initialSamplesCountInput_(nullptr),
      initialSamplesDensityInput_(nullptr),
      neighborhoodRadiusInput_(nullptr),
      segmentationL1_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    SliderWidget::create(initialSamplesCountInput_,
                         this,
                         nullptr,
                         SLOT(slotInitialSamplesCountFinalValue()),
                         tr("Number of initial samples"),
                         tr("Number of initial samples"),
                         tr("%"),
                         1,
                         1,
                         100,
                         parameters_.initialSamplesCount);

    RangeSliderWidget::create(initialSamplesDensityInput_,
                              this,
                              SLOT(slotInitialSamplesDensityMinimumValue()),
                              SLOT(slotInitialSamplesDensityMaximumValue()),
                              tr("Density range of initial samples"),
                              tr("Density range of initial samples"),
                              tr("%"),
                              1,
                              0,
                              100,
                              parameters_.initialSamplesDensityMinimum,
                              parameters_.initialSamplesDensityMaximum);

    RangeSliderWidget::create(neighborhoodRadiusInput_,
                              this,
                              SLOT(slotNeighborhoodRadiusMinimumValue()),
                              SLOT(slotNeighborhoodRadiusMaximumValue()),
                              tr("Neighborhood radius range"),
                              tr("Neighborhood radius range"),
                              tr("pt"),
                              1,
                              1,
                              10000,
                              parameters_.neighborhoodRadiusMinimum,
                              parameters_.neighborhoodRadiusMaximum);

    // Create layout with parameters.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(initialSamplesCountInput_);
    settingsLayout->addWidget(initialSamplesDensityInput_);
    settingsLayout->addWidget(neighborhoodRadiusInput_);

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
    parameters_.set(initialSamplesCountInput_->value(),
                    initialSamplesDensityInput_->minimumValue(),
                    initialSamplesDensityInput_->maximumValue(),
                    neighborhoodRadiusInput_->minimumValue(),
                    neighborhoodRadiusInput_->maximumValue());

    LOG_DEBUG(<< "Apply parameters <" << parameters_ << ">.");

    return segmentationL1_.applyParameters(parameters_);
}

bool SegmentationL1Window::next()
{
    LOG_DEBUG(<< "Compute the next step.");
    return segmentationL1_.next();
}

int SegmentationL1Window::progressPercent()
{
    return segmentationL1_.progressPercent();
}

void SegmentationL1Window::updateData()
{
    LOG_DEBUG(<< "Update data.");
    mainWindow_->update({Editor::TYPE_LAYER});
}

void SegmentationL1Window::slotInitialSamplesCountFinalValue()
{
    LOG_DEBUG(<< "New value for the number of initial samples <"
              << initialSamplesCountInput_->value() << ">.");
    emit signalParametersChanged();
}

void SegmentationL1Window::slotInitialSamplesDensityMinimumValue()
{
    LOG_DEBUG(<< "New value for minimum density of initial samples <"
              << initialSamplesDensityInput_->minimumValue() << ">.");
    emit signalParametersChanged();
}

void SegmentationL1Window::slotInitialSamplesDensityMaximumValue()
{
    LOG_DEBUG(<< "New value for maximum density of initial samples <"
              << initialSamplesDensityInput_->maximumValue() << ">.");
    emit signalParametersChanged();
}

void SegmentationL1Window::slotNeighborhoodRadiusMinimumValue()
{
    LOG_DEBUG(<< "New value for minimum neighborhood radius <"
              << neighborhoodRadiusInput_->minimumValue() << ">.");
    emit signalParametersChanged();
}

void SegmentationL1Window::slotNeighborhoodRadiusMaximumValue()
{
    LOG_DEBUG(<< "New value for maximum neighborhood radius <"
              << neighborhoodRadiusInput_->maximumValue() << ">.");
    emit signalParametersChanged();
}
