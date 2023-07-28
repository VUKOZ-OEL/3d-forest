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

#include <InfoDialog.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <RangeSliderWidget.hpp>
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
      infoDialog_(nullptr),
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

    RangeSliderWidget::create(elevationSlider_,
                              this,
                              nullptr,
                              nullptr,
                              tr("Minimal tree elevation"),
                              tr("Ignore all trees which are only outside"
                                 " of this elevation threshold."),
                              tr("%"),
                              1,
                              0,
                              100,
                              5,
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
    settingsLayout->addWidget(radiusSlider_);
    settingsLayout->addWidget(descriptorSlider_);
    settingsLayout->addWidget(elevationSlider_);
    settingsLayout->addWidget(groupSizeSlider_);
    settingsLayout->addStretch();

    // Buttons
    helpButton_ = new QPushButton(tr("Help"));
    helpButton_->setIcon(THEME_ICON("question"));
    connect(helpButton_, SIGNAL(clicked()), this, SLOT(slotHelp()));

    applyButton_ = new QPushButton(tr("Run"));
    applyButton_->setIcon(THEME_ICON("run"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(helpButton_);
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
    double elevationMin = static_cast<double>(elevationSlider_->minimumValue());
    double elevationMax = static_cast<double>(elevationSlider_->maximumValue());
    size_t groupSize = static_cast<size_t>(groupSizeSlider_->value());

    try
    {
        segmentation_.start(voxelSize,
                            descriptor,
                            radius,
                            elevationMin * 0.01,
                            elevationMax * 0.01,
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

void SegmentationWidget::slotHelp()
{
    QString t = "<h3>Segmentation Tool</h3>"
                "This tool identifies trees in point cloud. "
                "It uses updated algorithm which is specialized to classify "
                "LiDAR point clouds of complex natural forest environments. "
                "<br><br>"
                "<img src=':/segmentation/segmentation.png'/>"
                "<div>Example dataset with calculated segmentation.</div>"
                ""
                "<h3>Algorithm</h3>"
                "<ol>"
                "<li>Voxelize the dataset.</li>"
                "<li>Detect individual trunks by using search radius"
                " to connect voxels which have descriptor values above"
                " user provided threshold. Assign a unique layer value"
                " to each detected trunk.</li>"
                "<li>Repeat the following for all remaining voxels:"
                "<ol>"
                "<li>Start at the next unprocessed voxel. The position"
                " of this voxel is random because the voxels are ordered"
                " by multi-layer octal-tree. This voxel creates new"
                " voxel group.</li>"
                "<li>Find spanning tree of this voxel until a voxel with"
                " existing layer value is reached."
                " The spanning tree is calculated by iteratively appending"
                " the next nearest neighbor to the current voxel group.</li>"
                "<li>Set layer value of all voxels in this voxel group"
                " to layer value from terminating voxel. This connects"
                " spanning trees to trunks. Connected voxels are marked"
                " as processed.</li>"
                "</ol>"
                "</li>"
                "<li>Layer values from voxels are applied back to the"
                " dataset.</li>"
                "</ol>";

    if (!infoDialog_)
    {
        infoDialog_ = new InfoDialog(mainWindow_, 450, 450);
        infoDialog_->setWindowTitle(tr("Segmentation Help"));
        infoDialog_->setText(t);
    }

    infoDialog_->show();
    infoDialog_->raise();
    infoDialog_->activateWindow();
}
