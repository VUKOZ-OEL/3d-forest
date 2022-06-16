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

/** @file SegmentationWindow.hpp */

#ifndef SEGMENTATION_WINDOW_HPP
#define SEGMENTATION_WINDOW_HPP

#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>

#include <QDialog>

class MainWindow;
class SliderWidget;

class QPushButton;

/** Segmentation Window. */
class SegmentationWindow : public QDialog, public ThreadCallbackInterface
{
    Q_OBJECT

public:
    SegmentationWindow(MainWindow *mainWindow);
    virtual ~SegmentationWindow();

    virtual void threadProgress(bool finished);

public slots:
    void slotThread();

signals:
    void signalThread();

protected slots:
    void slotDistanceFinalValue();
    void slotThresholdFinalValue();

    void slotAccept();
    void slotReject();

protected:
    MainWindow *mainWindow_;

    SliderWidget *previewSizeInput_;
    SliderWidget *distanceInput_;
    SliderWidget *thresholdInput_;

    QPushButton *acceptButton_;
    QPushButton *rejectButton_;

    SegmentationThread segmentationThread_;

    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    void suspendThreads();
    void resumeThreads();
};

#endif /* SEGMENTATION_WINDOW_HPP */
