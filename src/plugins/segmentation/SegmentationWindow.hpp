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

#include <QDockWidget>

class MainWindow;

class QSpinBox;
class QSlider;
class QVBoxLayout;

/** Segmentation Window. */
class SegmentationWindow : public QDockWidget
{
    Q_OBJECT

public:
    SegmentationWindow(MainWindow *mainWindow);

protected slots:
    void slotDistanceFinalValue();
    void slotDistanceIntermediateValue(int v);

    void slotThresholdFinalValue();
    void slotThresholdIntermediateValue(int v);

    void slotApply();

protected:
    MainWindow *mainWindow_;
    QWidget *widget_;

    QWidget *distanceGroup_;
    QSlider *distanceSlider_;
    QSpinBox *distanceSpinBox_;

    QWidget *thresholdGroup_;
    QSlider *thresholdSlider_;
    QSpinBox *thresholdSpinBox_;

    void createInputSlider(QVBoxLayout *layout,
                           QWidget *&group,
                           QSlider *&slider,
                           QSpinBox *&spinBox,
                           const char *memberIntermediateValue,
                           const char *memberFinalValue,
                           const QString &text,
                           const QString &toolTip,
                           const QString &unitsList,
                           int step,
                           int min,
                           int max,
                           int value);
};

#endif /* SEGMENTATION_WINDOW_HPP */
