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

/** @file SegmentationWindow.cpp */

#include <Log.hpp>
#include <MainWindow.hpp>
#include <RangeSliderWidget.hpp>
#include <SegmentationWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "SegmentationWindow"
#define ICON(name) (ThemeIcon(":/segmentation/", name))

#define SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN 1
#define SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX 100

SegmentationWindow::SegmentationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      segmentationThread_(&mainWindow->editor())
{
    LOG_DEBUG(<< "Called.");

    // Voxel size
    SliderWidget::create(voxelSizeInput_,
                         this,
                         nullptr,
                         SLOT(slotVoxelSizeFinalValue()),
                         tr("Voxel size"),
                         tr("The automated segmentation creates voxel"
                            "\ngrid through the whole point cloud."
                            "\nVoxel size should be small enough so that"
                            "\nmost voxels contain only points from a"
                            " single tree."),
                         tr("pt"),
                         1,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX);

    // Maximum elevation
    RangeSliderWidget::create(seedElevationInput_,
                              this,
                              SLOT(slotSeedElevationMinimumValue()),
                              SLOT(slotSeedElevationMaximumValue()),
                              tr("Elevation range of tree base"),
                              tr("Elevation range of points which"
                                 "\nare used to start segmentation."),
                              tr("%"),
                              1,
                              1,
                              25,
                              4,
                              6);

    // Minimum height of tree
    SliderWidget::create(treeHeightInput_,
                         this,
                         nullptr,
                         SLOT(slotTreeHeightFinalValue()),
                         tr("Minimum height of tree"),
                         tr("Minimum height of cluster created from"
                            "\nnearby points to mark it as a new tree"),
                         tr("%"),
                         1,
                         1,
                         100,
                         10);

    // Search radius
    SliderWidget::create(searchRadiusInput_,
                         this,
                         nullptr,
                         SLOT(slotSearchRadiusFinalValue()),
                         tr("Search radius"),
                         tr("Search radius"),
                         tr("pt"),
                         1,
                         1,
                         10000,
                         5000);

    // Neighbor points
    SliderWidget::create(neighborPointsInput_,
                         this,
                         nullptr,
                         SLOT(slotNeighborPointsFinalValue()),
                         tr("Neighbor points"),
                         tr("Neighbor points"),
                         tr("cnt"),
                         1,
                         1,
                         10000,
                         10);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(voxelSizeInput_);
    settingsLayout->addWidget(seedElevationInput_);
    settingsLayout->addWidget(treeHeightInput_);
    settingsLayout->addWidget(searchRadiusInput_);
    settingsLayout->addWidget(neighborPointsInput_);
    settingsLayout->addStretch();

    // apply/cancel buttons
    acceptButton_ = new QPushButton(tr("Apply"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Main layout
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(settingsLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Dialog
    setWindowTitle(tr("Segmentation"));
    setWindowIcon(ICON("forest"));
    setMaximumHeight(height());
    setModal(true);

    // Connect worker thread to gui thread
    connect(this,
            SIGNAL(signalThread(bool, int)),
            this,
            SLOT(slotThread(bool, int)),
            Qt::QueuedConnection);

    segmentationThread_.setCallback(this);
    segmentationThread_.create();
}

SegmentationWindow::~SegmentationWindow()
{
    LOG_DEBUG(<< "Called.");
    segmentationThread_.stop();
}

void SegmentationWindow::updateRange()
{
    LOG_DEBUG(<< "Called.");
    const Editor *editor = segmentationThread_.editor();
    Box<double> boundary = editor->clipBoundary();
    double max = boundary.maximumLength() * 0.1;

    int length = static_cast<int>(max);
    if (length < SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN)
    {
        length = SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN;
    }

    voxelSizeInput_->blockSignals(true);
    voxelSizeInput_->setMaximum(length);
    voxelSizeInput_->setValue(length);
    voxelSizeInput_->blockSignals(false);
}

void SegmentationWindow::slotVoxelSizeFinalValue()
{
    LOG_DEBUG(<< "Input value <" << voxelSizeInput_->value() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotSeedElevationMinimumValue()
{
    LOG_DEBUG(<< "Input value <" << seedElevationInput_->minimumValue()
              << ">.");
    resumeThreads();
}

void SegmentationWindow::slotSeedElevationMaximumValue()
{
    LOG_DEBUG(<< "Input value <" << seedElevationInput_->maximumValue()
              << ">.");
    resumeThreads();
}

void SegmentationWindow::slotTreeHeightFinalValue()
{
    LOG_DEBUG(<< "Input value <" << treeHeightInput_->value() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotSearchRadiusFinalValue()
{
    LOG_DEBUG(<< "Input value <" << treeHeightInput_->value() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotNeighborPointsFinalValue()
{
    LOG_DEBUG(<< "Input value <" << neighborPointsInput_->value() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotAccept()
{
    LOG_DEBUG(<< "Called.");
    close();
    setResult(QDialog::Accepted);
}

void SegmentationWindow::slotReject()
{
    LOG_DEBUG(<< "Called.");
    close();
    setResult(QDialog::Rejected);
}

void SegmentationWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG(<< "Called.");
    QDialog::showEvent(event);
    mainWindow_->suspendThreads();
    updateRange();
    resumeThreads();
}

void SegmentationWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG(<< "Called.");
    suspendThreads();
    mainWindow_->resumeThreads();
    QDialog::closeEvent(event);
}

void SegmentationWindow::threadProgress(bool finished)
{
    LOG_DEBUG(<< "Called with parameter finished <" << finished << ">.");
    // in worker thread: notify gui thread
    emit signalThread(finished, segmentationThread_.progressPercent());
}

void SegmentationWindow::slotThread(bool finished, int progressPercent)
{
    LOG_DEBUG(<< "Called with parameter finished <" << finished
              << "> progress <" << progressPercent << ">.");
    // in gui thread: update visualization
    mainWindow_->setStatusProgressBarPercent(progressPercent);

    if (finished)
    {
        LOG_TRACE_UNKNOWN(<< "Thread finished.");
        mainWindow_->update({Editor::TYPE_LAYER, Editor::TYPE_DESCRIPTOR});
    }
}

void SegmentationWindow::suspendThreads()
{
    LOG_DEBUG(<< "Called.");
    // in gui thread: cancel task in worker thread
    segmentationThread_.cancel();
    mainWindow_->setStatusProgressBarPercent(0);
}

void SegmentationWindow::resumeThreads()
{
    LOG_DEBUG(<< "Resume thread with parameters voxelSize <"
              << voxelSizeInput_->value() << "> seedElevationMinimum <"
              << seedElevationInput_->minimumValue()
              << "> seedElevationMaximum <"
              << seedElevationInput_->maximumValue() << "> treeHeight <"
              << treeHeightInput_->value() << "> searchRadius <"
              << searchRadiusInput_->value() << "> neighborPoints <"
              << neighborPointsInput_->value() << ">.");

    // in gui thread: start new task in worker thread
    segmentationThread_.start(voxelSizeInput_->value(),
                              seedElevationInput_->minimumValue(),
                              seedElevationInput_->maximumValue(),
                              treeHeightInput_->value(),
                              searchRadiusInput_->value(),
                              neighborPointsInput_->value());

    mainWindow_->setStatusProgressBarPercent(0);
}
