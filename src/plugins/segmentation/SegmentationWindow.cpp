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
#include <SegmentationWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/segmentation/", name))

#define MODULE_NAME "SegmentationWindow"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

#define SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN 1
#define SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX 100

SegmentationWindow::SegmentationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      segmentationThread_(&mainWindow->editor())
{
    LOG_DEBUG_LOCAL("");

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;

    // Input widgets
    SliderWidget::create(previewSizeInput_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Preview size"),
                         tr("Preview size"),
                         tr("%"),
                         1,
                         1,
                         100,
                         10);

    settingsLayout->addWidget(previewSizeInput_);
    previewSizeInput_->setDisabled(true);

    SliderWidget::create(voxelSizeInput_,
                         this,
                         nullptr,
                         SLOT(slotVoxelSizeFinalValue()),
                         tr("Voxel size"),
                         tr("The automated segmentation creates voxel"
                            " grid through the whole point cloud."
                            "\nVoxel size should be small enough so that"
                            " each voxel contains only one characteristic"
                            "\nthat best fits the data inside voxel volume."
                            " Each voxel should contain at least 3 points."),
                         tr("pt"),
                         1,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MIN,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX,
                         SEGMENTATION_WINDOW_VOXEL_SIZE_DEFAULT_MAX);

    settingsLayout->addWidget(voxelSizeInput_);

    SliderWidget::create(thresholdInput_,
                         this,
                         nullptr,
                         SLOT(slotThresholdFinalValue()),
                         tr("Use descriptor values above"),
                         tr("The range represents the lower limit of the"
                            " used voxels for segmentation."
                            "\nFor example, if the input threshold value"
                            " is 70, then only the voxels whose descriptor"
                            "\nvalue is at 70% of the actual value range"
                            " or higher are used for the tree extraction."),
                         tr("%"),
                         1,
                         0,
                         100,
                         30);

    settingsLayout->addWidget(thresholdInput_);

    SliderWidget::create(voxelsInElementInput_,
                         this,
                         nullptr,
                         SLOT(slotVoxelsInElementFinalValue()),
                         tr("Voxels per element"),
                         tr("Minimal number of voxels in an element"),
                         tr("count"),
                         1,
                         1,
                         999,
                         150);

    settingsLayout->addWidget(voxelsInElementInput_);

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
    LOG_DEBUG_LOCAL("");
    segmentationThread_.stop();
}

void SegmentationWindow::updateRange()
{
    LOG_DEBUG_LOCAL("");
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
    LOG_DEBUG_LOCAL("value <" << voxelSizeInput_->value() << ">");
    resumeThreads();
}

void SegmentationWindow::slotThresholdFinalValue()
{
    LOG_DEBUG_LOCAL("value <" << thresholdInput_->value() << ">");
    resumeThreads();
}

void SegmentationWindow::slotVoxelsInElementFinalValue()
{
    LOG_DEBUG_LOCAL("value <" << voxelsInElementInput_->value() << ">");
    resumeThreads();
}

void SegmentationWindow::slotAccept()
{
    LOG_DEBUG_LOCAL("");
    close();
    setResult(QDialog::Accepted);
}

void SegmentationWindow::slotReject()
{
    LOG_DEBUG_LOCAL("");
    close();
    setResult(QDialog::Rejected);
}

void SegmentationWindow::showEvent(QShowEvent *event)
{
    LOG_DEBUG_LOCAL("");
    QDialog::showEvent(event);
    mainWindow_->suspendThreads();
    updateRange();
    resumeThreads();
}

void SegmentationWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG_LOCAL("");
    suspendThreads();
    mainWindow_->resumeThreads();
    QDialog::closeEvent(event);
}

void SegmentationWindow::threadProgress(bool finished)
{
    LOG_DEBUG_LOCAL("finished <" << finished << ">");
    // in worker thread: notify gui thread
    emit signalThread(finished, segmentationThread_.progressPercent());
}

void SegmentationWindow::slotThread(bool finished, int progressPercent)
{
    LOG_DEBUG_LOCAL("finished <" << finished << "> progress <"
                                 << progressPercent << ">");
    // in gui thread: update visualization
    mainWindow_->setStatusProgressBarPercent(progressPercent);
    (void)finished;
    if (finished)
    {
        LOG_UPDATE_VIEW(MODULE_NAME, "finished");
        mainWindow_->update({"Layers", "Descriptor"});
    }
}

void SegmentationWindow::suspendThreads()
{
    LOG_DEBUG_LOCAL("");
    // in gui thread: cancel task in worker thread
    segmentationThread_.cancel();
    mainWindow_->setStatusProgressBarPercent(0);
}

void SegmentationWindow::resumeThreads()
{
    LOG_DEBUG_LOCAL("");
    // in gui thread: start new task in worker thread
    segmentationThread_.start(voxelSizeInput_->value(),
                              thresholdInput_->value(),
                              voxelsInElementInput_->value());
    mainWindow_->setStatusProgressBarPercent(0);
}
