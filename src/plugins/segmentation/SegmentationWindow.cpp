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
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QShowEvent>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/segmentation/", name))
//#define LOG_DEBUG_LOCAL(msg)
#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationWindow", msg)

SegmentationWindow::SegmentationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG_LOCAL("");

    // Layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;

    // Widgets
    createInputSlider(settingsLayout,
                      distanceGroup_,
                      distanceSlider_,
                      distanceSpinBox_,
                      this,
                      SLOT(slotDistanceIntermediateValue(int)),
                      SLOT(slotDistanceFinalValue()),
                      tr("Voxel size"),
                      tr("The automated segmentation creates voxel"
                         " grid through the whole point cloud."
                         "\nVoxel size should be small enough so that"
                         " each voxel contains only one characteristic"
                         "\nthat best fits the data inside voxel volume."
                         " Each voxel should contain at least 3 points."),
                      tr("pt"),
                      1,
                      1,
                      100,
                      10);

    createInputSlider(settingsLayout,
                      thresholdGroup_,
                      thresholdSlider_,
                      thresholdSpinBox_,
                      this,
                      SLOT(slotThresholdIntermediateValue(int)),
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
                      70);

    acceptButton_ = new QPushButton(tr("Apply"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    // Layout
    settingsLayout->addStretch();

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

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
            SIGNAL(signalThread()),
            this,
            SLOT(slotThread()),
            Qt::QueuedConnection);

    segmentationThread_.setCallback(this);
    segmentationThread_.create();
}

SegmentationWindow::~SegmentationWindow()
{
    LOG_DEBUG_LOCAL("");
    segmentationThread_.stop();
}

void SegmentationWindow::slotDistanceFinalValue()
{
    LOG_DEBUG_LOCAL("value <" << distanceSlider_->value() << ">");
    resumeThreads();
}

void SegmentationWindow::slotDistanceIntermediateValue(int v)
{
    LOG_DEBUG_LOCAL("value <" << v << ">");
    QObject *obj = sender();
    if (obj == distanceSlider_)
    {
        LOG_DEBUG_LOCAL("slider value");
        distanceSpinBox_->blockSignals(true);
        distanceSpinBox_->setValue(v);
        distanceSpinBox_->blockSignals(false);
    }
    else if (obj == distanceSpinBox_)
    {
        LOG_DEBUG_LOCAL("spin box value");
        distanceSlider_->blockSignals(true);
        distanceSlider_->setValue(v);
        distanceSlider_->blockSignals(false);
    }
}

void SegmentationWindow::slotThresholdFinalValue()
{
    LOG_DEBUG_LOCAL("value <" << thresholdSlider_->value() << ">");
}

void SegmentationWindow::slotThresholdIntermediateValue(int v)
{
    LOG_DEBUG_LOCAL("value <" << v << ">");
    QObject *obj = sender();
    if (obj == thresholdSlider_)
    {
        LOG_DEBUG_LOCAL("slider value");
        thresholdSpinBox_->setValue(v);
    }
    else if (obj == thresholdSpinBox_)
    {
        LOG_DEBUG_LOCAL("spin box value");
        thresholdSlider_->setValue(v);
    }
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
    resumeThreads();
}

void SegmentationWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG_LOCAL("");
    suspendThreads();
    QDialog::closeEvent(event);
}

void SegmentationWindow::threadProgress(bool finished)
{
    (void)finished;
    LOG_DEBUG_LOCAL("finished=" << finished);
    // in worker thread: notify gui thread
    emit signalThread();
}

void SegmentationWindow::slotThread()
{
    LOG_DEBUG_LOCAL("");
    // in gui thread: update visualization
}

void SegmentationWindow::suspendThreads()
{
    LOG_DEBUG_LOCAL("");
    // in gui thread: cancel task in worker thread
    segmentationThread_.cancel();
}

void SegmentationWindow::resumeThreads()
{
    LOG_DEBUG_LOCAL("");
    // in gui thread: start new task in worker thread
    segmentationThread_.setup();
}

void SegmentationWindow::createInputSlider(QVBoxLayout *layout,
                                           QWidget *&group,
                                           QSlider *&slider,
                                           QSpinBox *&spinBox,
                                           const QObject *receiver,
                                           const char *memberIntermediateValue,
                                           const char *memberFinalValue,
                                           const QString &text,
                                           const QString &toolTip,
                                           const QString &unitsList,
                                           int step,
                                           int min,
                                           int max,
                                           int value)
{
    // Description Name
    QLabel *label = new QLabel(text);

    // Description Tool Tip
    QLabel *help = new QLabel;
    help->setToolTip(toolTip);
    ThemeIcon helpIcon(":/gui/", "question");
    help->setPixmap(helpIcon.pixmap(MainWindow::ICON_SIZE_TEXT));

    // Description Units
    QComboBox *units = new QComboBox;
    units->addItem(unitsList);

    // Description Layout
    QHBoxLayout *descriptionLayout = new QHBoxLayout;
    descriptionLayout->addWidget(label);
    descriptionLayout->addWidget(help);
    descriptionLayout->addStretch();
    descriptionLayout->addWidget(units);

    // Value Slider
    slider = new QSlider;
    slider->setRange(min, max);
    slider->setValue(value);
    slider->setSingleStep(step);
    slider->setOrientation(Qt::Horizontal);
    connect(slider,
            SIGNAL(valueChanged(int)),
            receiver,
            memberIntermediateValue);
    connect(slider, SIGNAL(sliderReleased()), receiver, memberFinalValue);

    // Value SpinBox
    spinBox = new QSpinBox;
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSingleStep(step);
    connect(spinBox,
            SIGNAL(valueChanged(int)),
            receiver,
            memberIntermediateValue);
    connect(spinBox, SIGNAL(editingFinished()), receiver, memberFinalValue);

    // Value Layout
    QHBoxLayout *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(slider);
    valueLayout->addWidget(spinBox);

    // Group Description and Value
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupLayout->addLayout(descriptionLayout);
    groupLayout->addLayout(valueLayout);

    group = new QWidget;
    group->setLayout(groupLayout);

    // Add Group to Main Layout
    layout->addWidget(group);
}
