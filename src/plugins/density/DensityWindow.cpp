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

/** @file DensityWindow.cpp */

#include <DensityConstants.hpp>
#include <DensityWindow.hpp>
#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QVBoxLayout>

#define LOG_MODULE_NAME "DensityWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/density/", name))

DensityWindow::DensityWindow(MainWindow *mainWindow)
    : AlgorithmWidgetInterface(mainWindow, ICON("density"), tr(DENSITY_NAME)),
      mainWindow_(mainWindow),
      neighborhoodRadiusInput_(nullptr),
      parameters_(),
      densityAlgorithm_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    SliderWidget::create(neighborhoodRadiusInput_,
                         this,
                         nullptr,
                         SLOT(slotParametersChanged()),
                         tr("Neighborhood Radius"),
                         tr("Neighborhood Radius"),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         parameters_.neighborhoodRadius);

    // Create layout with parameters.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(neighborhoodRadiusInput_);

    // Create widget layout.
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(settingsLayout);
    widgetLayout->addStretch();

    mainLayout_->addLayout(widgetLayout);
    setLayout(mainLayout_);
}

DensityWindow::~DensityWindow()
{
    LOG_DEBUG(<< "Destroy.");
}

bool DensityWindow::applyParameters()
{
    parameters_.set(neighborhoodRadiusInput_->value());

    LOG_DEBUG(<< "Apply parameters <" << parameters_ << ">.");

    return densityAlgorithm_.applyParameters(parameters_);
}

bool DensityWindow::next()
{
    LOG_DEBUG(<< "Compute the next step.");
    return densityAlgorithm_.next();
}

void DensityWindow::progress(size_t &nTasks,
                             size_t &iTask,
                             double &percent) const
{
    densityAlgorithm_.progress(nTasks, iTask, percent);
}

void DensityWindow::updateData()
{
    LOG_DEBUG(<< "Update data.");
    mainWindow_->update({Editor::TYPE_DESCRIPTOR});
}

void DensityWindow::slotParametersChanged()
{
    LOG_DEBUG(<< "New value for some input parameter.");
    emit signalParametersChanged();
}
