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

/** @file DoubleSliderWidget.hpp */

#ifndef DOUBLE_SLIDER_WIDGET_HPP
#define DOUBLE_SLIDER_WIDGET_HPP

// Include Qt.
#include <QWidget>
class QDoubleSpinBox;
class QSlider;
class QVBoxLayout;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Slider Widget with values in double. */
class EXPORT_GUI DoubleSliderWidget : public QWidget
{
    Q_OBJECT

public:
    /** Slider layout. */
    enum Layout
    {
        LAYOUT_SLIDER_BESIDE_LABEL,
        LAYOUT_SLIDER_UNDER_LABEL
    };

    DoubleSliderWidget();

    static void create(DoubleSliderWidget *&outputWidget,
                       const QObject *receiver,
                       const char *memberIntermediateValue,
                       const char *memberFinalValue,
                       const QString &text,
                       const QString &toolTip,
                       const QString &unitsList,
                       double step,
                       double min,
                       double max,
                       double value,
                       Layout layout = LAYOUT_SLIDER_UNDER_LABEL);

    double value() const;
    double minimum() const;
    double maximum() const;

    void setValue(double value);
    void setMinimum(double min);
    void setMaximum(double max);

    void blockSignals(bool block);

signals:
    void signalIntermediateValue(double v);
    void signalFinalValue();

protected slots:
    void slotIntermediateValue(int v);
    void slotIntermediateValue(double v);
    void slotFinalValue();

protected:
    QSlider *slider_;
    QDoubleSpinBox *spinBox_;
    double minimumValue_;
    double maximumValue_;
};

#include <WarningsEnable.hpp>

#endif /* DOUBLE_SLIDER_WIDGET_HPP */
