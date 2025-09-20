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

/** @file ComputeCrownVolumeWidget.cpp */

// Include 3D Forest.
#include <ComputeCrownVolumeWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ComputeCrownVolumeWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeCrownVolumeResources/", name))

ComputeCrownVolumeWidget::ComputeCrownVolumeWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      action_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Voxel size.
    DoubleSliderWidget::create(
        voxelSizeSlider_,
        this,
        nullptr,
        nullptr,
        tr("Voxel size"),
        tr("Higher voxel size values affect the quality of the results but "
           "speed up computation and reduce disk space usage."),
        tr("m"),
        0.01,
        0.01,
        1.0,
        parameters_.voxelSize);

    voxelSizeSlider_->setTargetProduct(1.0);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSizeSlider_);
    settingsLayout->addStretch();

    // Buttons.
    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run").icon());
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget.
    setLayout(mainLayout);
}

void ComputeCrownVolumeWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    action_.clear();
    QWidget::hideEvent(event);
}

void ComputeCrownVolumeWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.voxelSize = voxelSizeSlider_->value();

    try
    {
        action_.start(parameters_);

        ProgressDialog::run(mainWindow_, "Computing Volume", &action_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_SEGMENT});
}
