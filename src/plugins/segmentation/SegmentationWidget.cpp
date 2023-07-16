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

/** @file SegmentationWidget.cpp */

#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <SegmentationWidget.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "SegmentationWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/segmentation/", name))

SegmentationWidget::SegmentationWidget(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow),
      segmentation_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    SliderWidget::create(voxelSizeSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Voxel radius"),
                         tr("Voxel radius to speed up computation."),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         100);

    SliderWidget::create(descriptorSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Wood descriptor threshold"),
                         tr("Wood descriptor threshold"),
                         tr("%"),
                         1,
                         0,
                         100,
                         25);

    SliderWidget::create(radiusSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Neighborhood radius"),
                         tr("Neighborhood radius to search for"
                            " points which belong to the same tree."),
                         tr("pt"),
                         1,
                         1,
                         1000,
                         250);

    SliderWidget::create(elevationSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Minimal tree elevation"),
                         tr("Ignore all trees which start below this"
                            " elevation threshold."),
                         tr("%"),
                         1,
                         0,
                         100,
                         20);

    SliderWidget::create(groupSizeSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Minimal points in tree"),
                         tr("Minimal number of points in tree"),
                         tr("count"),
                         1,
                         1,
                         1000,
                         10);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(new QLabel(tr("This tool requires pre-computed"
                                            " elevation and\ndescriptor values"
                                            " to get the best results.")));
    settingsLayout->addWidget(voxelSizeSlider_);
    settingsLayout->addWidget(descriptorSlider_);
    settingsLayout->addWidget(radiusSlider_);
    settingsLayout->addWidget(elevationSlider_);
    settingsLayout->addWidget(groupSizeSlider_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Widget
    setLayout(mainLayout);
}

void SegmentationWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG(<< "Hide.");
    segmentation_.clear();
    QWidget::hideEvent(event);
}

void SegmentationWidget::slotApply()
{
    LOG_DEBUG(<< "Apply.");

    mainWindow_->suspendThreads();

    double voxelSize = static_cast<double>(voxelSizeSlider_->value());
    double descriptor = static_cast<double>(descriptorSlider_->value()) * 0.01;
    double radius = static_cast<double>(radiusSlider_->value());
    double elevation = static_cast<double>(elevationSlider_->value()) * 0.01;
    size_t groupSize = static_cast<size_t>(groupSizeSlider_->value());

    try
    {
        segmentation_.start(voxelSize,
                            descriptor,
                            radius,
                            elevation,
                            groupSize);

        ProgressDialog::run(mainWindow_,
                            "Computing Segmentation",
                            &segmentation_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->update({Editor::TYPE_LAYER});
}
