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

/** @file SliderWidget.hpp */

#ifndef SLIDER_WIDGET_HPP
#define SLIDER_WIDGET_HPP

#include <QWidget>

class QSpinBox;
class QSlider;
class QVBoxLayout;

/** Slider Widget. */
class SliderWidget : public QWidget
{
    Q_OBJECT

public:
    SliderWidget();

    static void create(SliderWidget *&outputWidget,
                       const QObject *receiver,
                       const char *memberIntermediateValue,
                       const char *memberFinalValue,
                       const QString &text,
                       const QString &toolTip,
                       const QString &unitsList,
                       int step,
                       int min,
                       int max,
                       int value);

    int value();

    void setValue(int value);
    void setMaximum(int max);

    void blockSignals(bool block);

signals:
    void signalIntermediateValue(int v);
    void signalFinalValue();

protected slots:
    void slotIntermediateValue(int v);
    void slotFinalValue();

protected:
    QSlider *slider_;
    QSpinBox *spinBox_;
};

#endif /* SLIDER_WIDGET_HPP */
