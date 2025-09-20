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

/** @file ComputeHullWidget.cpp */

// Include 3D Forest.
#include <ComputeHullWidget.hpp>
#include <DoubleSliderWidget.hpp>
#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ComputeHullWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeHullResources/", name))

ComputeHullWidget::ComputeHullWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      action_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Hull options.
    computeConvexHullCheckBox_ = new QCheckBox;
    computeConvexHullCheckBox_->setChecked(false);
    computeConvexHullCheckBox_->setText(tr("Convex hull"));

    computeConvexHullProjectionCheckBox_ = new QCheckBox;
    computeConvexHullProjectionCheckBox_->setChecked(false);
    computeConvexHullProjectionCheckBox_->setText(tr("Convex hull projection"));

    computeConcaveHullCheckBox_ = new QCheckBox;
    computeConcaveHullCheckBox_->setChecked(true);
    computeConcaveHullCheckBox_->setText(tr("Concave hull (alpha shape)"));

    computeConcaveHullProjectionCheckBox_ = new QCheckBox;
    computeConcaveHullProjectionCheckBox_->setChecked(true);
    computeConcaveHullProjectionCheckBox_->setText(
        tr("Concave hull projection"));

    // Hull type options group.
    QVBoxLayout *typeOptionsVBoxLayout = new QVBoxLayout;
    typeOptionsVBoxLayout->addWidget(computeConvexHullCheckBox_);
    typeOptionsVBoxLayout->addWidget(computeConvexHullProjectionCheckBox_);
    typeOptionsVBoxLayout->addWidget(computeConcaveHullCheckBox_);
    typeOptionsVBoxLayout->addWidget(computeConcaveHullProjectionCheckBox_);

    QGroupBox *typeOptionsGroupBox = new QGroupBox(tr("Compute hull"));
    typeOptionsGroupBox->setLayout(typeOptionsVBoxLayout);

    QGridLayout *typeOptionsGroupBoxLayout = new QGridLayout;
    typeOptionsGroupBoxLayout->addWidget(typeOptionsGroupBox, 0, 1);

    // Concave hull alpha.
    findOptimalAlphaCheckBox_ = new QCheckBox;
    findOptimalAlphaCheckBox_->setText(tr("Find optimal alpha"));
    findOptimalAlphaCheckBox_->setChecked(parameters_.findOptimalAlpha);
    connect(findOptimalAlphaCheckBox_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(slotFindOptimalAlphaChanged(int)));

    // If your data points are spaced ~1 unit apart, α = 0.1 will include only
    // very local features. If points are spaced ~100 units apart, you might
    // need α = 50+ to see a connected shape.

    DoubleSliderWidget::create(
        alphaSlider_,
        this,
        nullptr,
        nullptr,
        tr("Alpha (sphere radius)"),
        tr("Controls the level of detail in the shape reconstruction. "
           "Smaller values - shapes that follow the data closely. "
           "Larger values - produce smoother, simpler shapes or "
           "the convex hull."),
        tr("m"),
        1.0,
        1.0,
        1000.0,
        parameters_.alpha);

    alphaSlider_->setEnabled(!findOptimalAlphaCheckBox_->isChecked());

    // Voxel radius.
    DoubleSliderWidget::create(
        voxelRadiusSlider_,
        this,
        nullptr,
        nullptr,
        tr("Voxel radius"),
        tr("Higher voxel radius values affect the quality of the results but "
           "speed up computation and reduce disk space usage."),
        tr("m"),
        0.01,
        0.01,
        1.0,
        parameters_.voxelRadius);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addLayout(typeOptionsGroupBoxLayout);
    settingsLayout->addWidget(findOptimalAlphaCheckBox_);
    settingsLayout->addWidget(alphaSlider_);
    settingsLayout->addWidget(voxelRadiusSlider_);
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

void ComputeHullWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    action_.clear();
    QWidget::hideEvent(event);
}

void ComputeHullWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    parameters_.computeConvexHull = computeConvexHullCheckBox_->isChecked();
    parameters_.computeConvexHullProjection =
        computeConvexHullProjectionCheckBox_->isChecked();
    parameters_.computeConcaveHull = computeConcaveHullCheckBox_->isChecked();
    parameters_.computeConcaveHullProjection =
        computeConcaveHullProjectionCheckBox_->isChecked();

    parameters_.findOptimalAlpha = findOptimalAlphaCheckBox_->isChecked();

    if (parameters_.findOptimalAlpha)
    {
        parameters_.alpha = -1.0;
    }
    else
    {
        double alpha = alphaSlider_->value();
        double ppm = mainWindow_->editor()
                         .settings()
                         .unitsSettings()
                         .pointsPerMeter()[0];

        parameters_.alpha = (alpha * alpha) * ppm;
    }

    parameters_.voxelRadius = voxelRadiusSlider_->value();

    try
    {
        action_.start(parameters_);

        ProgressDialog::run(mainWindow_, "Computing Hull", &action_);
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

void ComputeHullWidget::slotFindOptimalAlphaChanged(int index)
{
    (void)index;
    alphaSlider_->setEnabled(!findOptimalAlphaCheckBox_->isChecked());
}
