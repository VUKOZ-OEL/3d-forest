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

/** @file ComputeSkeletonWidget.cpp */

// Include 3D Forest.
#include <ComputeSkeletonWidget.hpp>
#include <DoubleSlider.hpp>
#include <InfoDialog.hpp>
#include <Application.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <HBoxLayout>
#include <PushButton>
#include <VBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ComputeSkeletonWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeSkeletonResources/", name))

ComputeSkeletonWidget::ComputeSkeletonWidget(Application *app)
    : Widget(),
      app_(app),
      action_(&app->editor())
{
    LOG_DEBUG(<< "Create.");

    // Voxel size.
    DoubleSlider::create(
        voxelSizeSlider_,
        this,
        nullptr,
        nullptr,
        tr("Voxel size"),
        tr("Higher voxel size values affect the quality of the results but "
           "speed up computation and reduce disk space usage."),
        tr("m"),
        0.1,
        0.1,
        1.0,
        parameters_.voxelSize);

    voxelSizeSlider_->setTargetProduct(1.0);

    // Settings layout.
    VBoxLayout *settingsLayout = new VBoxLayout;
    settingsLayout->addWidget(voxelSizeSlider_);
    settingsLayout->addStretch();

    // Buttons.
    applyButton_ = new PushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout.
    HBoxLayout *buttonsLayout = new HBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget.
    setLayout(mainLayout);
}

void ComputeSkeletonWidget::hideEvent(HideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    action_.clear();
    Widget::hideEvent(event);
}

void ComputeSkeletonWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    app_->suspendThreads();

    parameters_.voxelSize = voxelSizeSlider_->value();

    try
    {
        action_.start(parameters_);

        ProgressDialog::run(app_,
                            "Computing Compute Skeleton",
                            &action_);
    }
    catch (std::exception &e)
    {
        app_->showError(e.what());
    }
    catch (...)
    {
        app_->showError("Unknown error");
    }

    app_->update(this, {Editor::TYPE_SEGMENT});
}
