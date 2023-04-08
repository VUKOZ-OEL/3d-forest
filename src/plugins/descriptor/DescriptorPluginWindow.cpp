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

/** @file DescriptorPluginWindow.cpp */

#include <DescriptorPluginConstants.hpp>
#include <DescriptorPluginWindow.hpp>
#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "DescriptorPluginWindow"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/descriptor/", name))

DescriptorPluginWindow::DescriptorPluginWindow(MainWindow *mainWindow)
    : AlgorithmWidgetInterface(mainWindow,
                               ICON("descriptor"),
                               tr(DESCRIPTOR_PLUGIN_NAME)),
      mainWindow_(mainWindow),
      methodPcaRadioButton_(nullptr),
      methodDensityRadioButton_(nullptr),
      neighborhoodRadiusInput_(nullptr),
      parameters_(),
      densityAlgorithm_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Method
    methodPcaRadioButton_ = new QRadioButton(tr("PCA"));
    methodDensityRadioButton_ = new QRadioButton(tr("Density"));

    if (parameters_.method == DescriptorAlgorithmParameters::METHOD_PCA)
    {
        methodPcaRadioButton_->setChecked(true);
    }
    else
    {
        methodDensityRadioButton_->setChecked(true);
    }

    QVBoxLayout *methodVBoxLayout = new QVBoxLayout;
    methodVBoxLayout->addWidget(methodPcaRadioButton_);
    methodVBoxLayout->addWidget(methodDensityRadioButton_);

    QGroupBox *methodGroupBox = new QGroupBox(tr("Method"));
    methodGroupBox->setLayout(methodVBoxLayout);

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
    settingsLayout->addWidget(methodGroupBox);
    settingsLayout->addWidget(neighborhoodRadiusInput_);

    // Create widget layout.
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(settingsLayout);
    widgetLayout->addStretch();

    mainLayout_->addLayout(widgetLayout);
    setLayout(mainLayout_);
}

DescriptorPluginWindow::~DescriptorPluginWindow()
{
    LOG_DEBUG(<< "Destroy.");
}

bool DescriptorPluginWindow::applyParameters()
{
    DescriptorAlgorithmParameters::Method method;
    if (methodPcaRadioButton_->isChecked())
    {
        method = DescriptorAlgorithmParameters::METHOD_PCA;
    }
    else
    {
        method = DescriptorAlgorithmParameters::METHOD_DENSITY;
    }

    parameters_.set(neighborhoodRadiusInput_->value(), method);

    LOG_DEBUG(<< "Apply parameters <" << parameters_ << ">.");

    return densityAlgorithm_.applyParameters(parameters_);
}

bool DescriptorPluginWindow::next()
{
    LOG_DEBUG(<< "Compute the next step.");
    return densityAlgorithm_.next();
}

void DescriptorPluginWindow::progress(size_t &nTasks,
                                      size_t &iTask,
                                      double &percent) const
{
    densityAlgorithm_.progress(nTasks, iTask, percent);
}

void DescriptorPluginWindow::updateData()
{
    LOG_DEBUG(<< "Update data.");
    mainWindow_->update({Editor::TYPE_DESCRIPTOR});
}

void DescriptorPluginWindow::slotParametersChanged()
{
    LOG_DEBUG(<< "New value for some input parameter.");
    emit signalParametersChanged();
}
