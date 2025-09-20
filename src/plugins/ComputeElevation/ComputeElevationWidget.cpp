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

/** @file ComputeElevationWidget.cpp */

// Include 3D Forest.
#include <ComputeElevationWidget.hpp>
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
#define LOG_MODULE_NAME "ComputeElevationWidget"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ComputeElevationResources/", name))

ComputeElevationWidget::ComputeElevationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      infoDialog_(nullptr),
      elevation_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    DoubleSliderWidget::create(voxelRadiusSlider_,
                               this,
                               nullptr,
                               nullptr,
                               tr("Voxel radius"),
                               tr("Voxel radius"),
                               tr("m"),
                               0.01,
                               0.01,
                               1.0,
                               0.1);

    // Settings layout.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelRadiusSlider_);
    settingsLayout->addStretch();

    // Buttons.
    helpButton_ = new QPushButton(tr("Help"));
    helpButton_->setIcon(THEME_ICON("question").icon());
    connect(helpButton_, SIGNAL(clicked()), this, SLOT(slotHelp()));

    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run").icon());
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(helpButton_);
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

void ComputeElevationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    elevation_.clear();
    QWidget::hideEvent(event);
}

void ComputeElevationWidget::slotApply()
{
    LOG_DEBUG(<< "Compute elevation.");

    mainWindow_->suspendThreads();

    double voxelRadius = voxelRadiusSlider_->value();

    try
    {
        elevation_.start(voxelRadius);
        ProgressDialog::run(mainWindow_, "Compute Elevation", &elevation_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_ELEVATION});
}

void ComputeElevationWidget::slotHelp()
{
    QString t;
    t = "<h3>Compute Elevation Tool</h3>"
        "This tool calculates elevation of points above ground. "
        "It uses new algorithm which is specialized to classify "
        "LiDAR point clouds of complex natural forest environments. "
        "The algorithm uses 2D projection to deal with "
        "missing ground data in non scanned or obstructed parts. "
        "Elevation is additional point attribute added by 3D Forest. "
        "<br>"
        "This tool requires pre-computed ground classification."
        "<br><br>"
        "<img src=':/ComputeElevationResources/elevation.png'/>"
        "<div>Example dataset with calculated elevation.</div>"
        ""
        "<h3>Compute Elevation Algorithm</h3>"
        "<ol>"
        "<li>Voxelize all ground points from the dataset.</li>"
        "<li>Create 2D quad-tree spatial index from 3D voxels."
        " Two dimensional quad-tree is created by ignoring"
        " z coordinates.</li>"
        "<li>Iterate all non-ground points, use their (x, y)"
        " coordinates to find nearest neighbor in the ground"
        " quad-tree and set elevation as difference between z"
        " coordinates of nearest ground voxel and iterated point.</li>"
        "</ol>";

    if (!infoDialog_)
    {
        infoDialog_ = new InfoDialog(mainWindow_, 450, 450);
        infoDialog_->setWindowTitle(tr("Compute Elevation Help"));
        infoDialog_->setText(t);
    }

    infoDialog_->show();
    infoDialog_->raise();
    infoDialog_->activateWindow();
}
