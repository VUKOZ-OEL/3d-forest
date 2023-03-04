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

/** @file SliderWidget.cpp */

#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

SliderWidget::SliderWidget() : QWidget()
{
}

int SliderWidget::value() const
{
    return slider_->value();
}

int SliderWidget::minimum() const
{
    return slider_->minimum();
}

int SliderWidget::maximum() const
{
    return slider_->maximum();
}

void SliderWidget::setValue(int value)
{
    spinBox_->setValue(value);
    slider_->setValue(value);
}

void SliderWidget::setMinimum(int min)
{
    spinBox_->setMinimum(min);
    slider_->setMinimum(min);
}

void SliderWidget::setMaximum(int max)
{
    spinBox_->setMaximum(max);
    slider_->setMaximum(max);
}

void SliderWidget::blockSignals(bool block)
{
    spinBox_->blockSignals(block);
    slider_->blockSignals(block);
}

void SliderWidget::slotFinalValue()
{
    emit signalFinalValue();
}

void SliderWidget::slotIntermediateValue(int v)
{
    QObject *obj = sender();

    if (obj == slider_)
    {
        spinBox_->blockSignals(true);
        spinBox_->setValue(v);
        spinBox_->blockSignals(false);
    }
    else if (obj == spinBox_)
    {
        slider_->blockSignals(true);
        slider_->setValue(v);
        slider_->blockSignals(false);
    }

    emit signalIntermediateValue(v);
}

void SliderWidget::create(SliderWidget *&outputWidget,
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
    outputWidget = new SliderWidget();

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
    outputWidget->slider_ = new QSlider;
    QSlider *slider = outputWidget->slider_;
    slider->setRange(min, max);
    slider->setValue(value);
    slider->setSingleStep(step);
    slider->setOrientation(Qt::Horizontal);

    connect(slider,
            SIGNAL(valueChanged(int)),
            outputWidget,
            SLOT(slotIntermediateValue(int)));

    connect(slider,
            SIGNAL(sliderReleased()),
            outputWidget,
            SLOT(slotFinalValue()));

    if (memberIntermediateValue)
    {
        connect(outputWidget,
                SIGNAL(signalIntermediateValue(int)),
                receiver,
                memberIntermediateValue);
    }

    if (memberFinalValue)
    {
        connect(outputWidget,
                SIGNAL(signalFinalValue()),
                receiver,
                memberFinalValue);
    }

    // Value SpinBox
    outputWidget->spinBox_ = new QSpinBox;
    QSpinBox *spinBox = outputWidget->spinBox_;
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSingleStep(step);

    connect(spinBox,
            SIGNAL(valueChanged(int)),
            outputWidget,
            SLOT(slotIntermediateValue(int)));

    connect(spinBox,
            SIGNAL(editingFinished()),
            outputWidget,
            SLOT(slotFinalValue()));

    // Value Layout
    QHBoxLayout *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(slider);
    valueLayout->addWidget(spinBox);

    // Group Description and Value
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupLayout->addLayout(descriptionLayout);
    groupLayout->addLayout(valueLayout);

    outputWidget->setLayout(groupLayout);
}
