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

/** @file RangeSliderWidget.hpp */

#ifndef RANGE_SLIDER_WIDGET_HPP
#define RANGE_SLIDER_WIDGET_HPP

// Include Qt.
#include <QWidget>
class QSpinBox;
class QVBoxLayout;

// Include 3rd party.
class ctkRangeSlider;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Range Slider Widget. */
class EXPORT_GUI RangeSliderWidget : public QWidget
{
    Q_OBJECT

public:
    RangeSliderWidget();

    static void create(RangeSliderWidget *&outputWidget,
                       const QObject *receiver,
                       const char *memberIntermediateMinimumValue,
                       const char *memberIntermediateMaximumValue,
                       const QString &text,
                       const QString &toolTip,
                       const QString &unitsList,
                       int step,
                       int min,
                       int max,
                       int minValue,
                       int maxValue);

    void setMinimum(int min);
    void setMaximum(int max);

    void setMinimumValue(int value);
    int minimumValue();

    void setMaximumValue(int value);
    int maximumValue();

    void blockSignals(bool block);

signals:
    void signalIntermediateMinimumValue();
    void signalIntermediateMaximumValue();

protected slots:
    void slotIntermediateMinimumValue(int v);
    void slotIntermediateMaximumValue(int v);

protected:
    ctkRangeSlider *slider_;
    QSpinBox *minSpinBox_;
    QSpinBox *maxSpinBox_;
    int minimumValue_;
    int maximumValue_;
};

#include <WarningsEnable.hpp>

#endif /* RANGE_SLIDER_WIDGET_HPP */
