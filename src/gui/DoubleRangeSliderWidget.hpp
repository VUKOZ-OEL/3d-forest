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

/** @file DoubleRangeSliderWidget.hpp */

#ifndef DOUBLE_RANGE_SLIDER_WIDGET_HPP
#define DOUBLE_RANGE_SLIDER_WIDGET_HPP

#include <QWidget>
class QDoubleSpinBox;
class QVBoxLayout;

class ctkDoubleRangeSlider;

#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Range Slider Widget with values in double. */
class EXPORT_GUI DoubleRangeSliderWidget : public QWidget
{
    Q_OBJECT

public:
    DoubleRangeSliderWidget();

    static void create(DoubleRangeSliderWidget *&outputWidget,
                       const QObject *receiver,
                       const char *memberIntermediateMinimumValue,
                       const char *memberIntermediateMaximumValue,
                       const QString &text,
                       const QString &toolTip,
                       const QString &unitsList,
                       double step,
                       double min,
                       double max,
                       double minValue,
                       double maxValue);

    void setMinimum(double min);
    void setMaximum(double max);

    void setMinimumValue(double value);
    double minimumValue();

    void setMaximumValue(double value);
    double maximumValue();

    void blockSignals(bool block);

signals:
    void signalIntermediateMinimumValue();
    void signalIntermediateMaximumValue();

protected slots:
    void slotIntermediateMinimumValue(double v);
    void slotIntermediateMaximumValue(double v);

protected:
    ctkDoubleRangeSlider *slider_;
    QDoubleSpinBox *minSpinBox_;
    QDoubleSpinBox *maxSpinBox_;
    double minimumValue_;
    double maximumValue_;
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_RANGE_SLIDER_WIDGET_HPP */
