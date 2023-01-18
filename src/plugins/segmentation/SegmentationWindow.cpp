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

    // Widgets.
    SliderWidget::create(initialSamplesCountInput_,
                         this,
                         nullptr,
                         SLOT(slotInitialSamplesCountFinalValue()),
                         tr("Number of initial samples"),
                         tr("Number of initial samples"),
                         tr("%"),
                         1,
                         1,
                         100,
                         parameters_.initialSamplesCount);

    RangeSliderWidget::create(initialSamplesDensityInput_,
                              this,
                              SLOT(slotInitialSamplesDensityMinimumValue()),
                              SLOT(slotInitialSamplesDensityMaximumValue()),
                              tr("Density range of initial samples"),
                              tr("Density range of initial samples"),
                              tr("%"),
                              1,
                              0,
                              100,
                              parameters_.initialSamplesDensityMinimum,
                              parameters_.initialSamplesDensityMaximum);

    RangeSliderWidget::create(neighborhoodRadiusInput_,
                              this,
                              SLOT(slotNeighborhoodRadiusMinimumValue()),
                              SLOT(slotNeighborhoodRadiusMaximumValue()),
                              tr("Neighborhood radius range"),
                              tr("Neighborhood radius range"),
                              tr("pt"),
                              1,
                              1,
                              10000,
                              parameters_.neighborhoodRadiusMinimum,
                              parameters_.neighborhoodRadiusMaximum);

    // Create layout with parameters.
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(initialSamplesCountInput_);
    settingsLayout->addWidget(initialSamplesDensityInput_);
    settingsLayout->addWidget(neighborhoodRadiusInput_);
    settingsLayout->addStretch();

    // Add apply and cancel buttons.
    acceptButton_ = new QPushButton(tr("Apply"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Create main layout.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(settingsLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Create the dialog.
    setWindowTitle(tr("Segmentation"));
    setWindowIcon(ICON("forest"));
    setMaximumHeight(height());
    setModal(true);

    // Connect worker thread to gui thread.
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

void SegmentationWindow::slotInitialSamplesCountFinalValue()
{
    LOG_DEBUG(<< "New value for the number of initial samples <"
              << initialSamplesCountInput_->value() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotInitialSamplesDensityMinimumValue()
{
    LOG_DEBUG(<< "New value for minimum density of initial samples <"
              << initialSamplesDensityInput_->minimumValue() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotInitialSamplesDensityMaximumValue()
{
    LOG_DEBUG(<< "New value for maximum density of initial samples <"
              << initialSamplesDensityInput_->maximumValue() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotNeighborhoodRadiusMinimumValue()
{
    LOG_DEBUG(<< "New value for minimum neighborhood radius <"
              << neighborhoodRadiusInput_->minimumValue() << ">.");
    resumeThreads();
}

void SegmentationWindow::slotNeighborhoodRadiusMaximumValue()
{
    LOG_DEBUG(<< "New value for maximum neighborhood radius <"
              << neighborhoodRadiusInput_->maximumValue() << ">.");
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
    parameters_.initialSamplesCount = initialSamplesCountInput_->value();
    parameters_.initialSamplesDensityMinimum =
        initialSamplesDensityInput_->minimumValue();
    parameters_.initialSamplesDensityMaximum =
        initialSamplesDensityInput_->maximumValue();
    parameters_.neighborhoodRadiusMinimum =
        neighborhoodRadiusInput_->minimumValue();
    parameters_.neighborhoodRadiusMaximum =
        neighborhoodRadiusInput_->maximumValue();

    LOG_DEBUG(<< "Resume thread with parameters initialSamplesCount <"
              << parameters_.initialSamplesCount
              << "> initialSamplesDensityMinimum <"
              << parameters_.initialSamplesDensityMinimum
              << "> initialSamplesDensityMaximum <"
              << parameters_.initialSamplesDensityMaximum
              << "> neighborhoodRadiusMinimum <"
              << parameters_.neighborhoodRadiusMinimum
              << "> neighborhoodRadiusMaximum <"
              << parameters_.neighborhoodRadiusMaximum << ">.");

    // in gui thread: start new task in worker thread
    segmentationThread_.restart(parameters_);

    mainWindow_->setStatusProgressBarPercent(0);
}
