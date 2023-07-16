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

/** @file DoubleSliderWidget.cpp */

#include <DoubleSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "DoubleSliderWidget"
#include <Log.hpp>

DoubleSliderWidget::DoubleSliderWidget() : QWidget()
{
}

double DoubleSliderWidget::value() const
{
    return static_cast<double>(slider_->value());
}

double DoubleSliderWidget::minimum() const
{
    return static_cast<double>(slider_->minimum());
}

double DoubleSliderWidget::maximum() const
{
    return static_cast<double>(slider_->maximum());
}

void DoubleSliderWidget::setValue(double value)
{
    int v = static_cast<int>(value);
    spinBox_->setValue(v);
    slider_->setValue(v);
}

void DoubleSliderWidget::setMinimum(double min)
{
    int v = static_cast<int>(min);
    spinBox_->setMinimum(v);
    slider_->setMinimum(v);
}

void DoubleSliderWidget::setMaximum(double max)
{
    int v = static_cast<int>(max);
    spinBox_->setMaximum(v);
    slider_->setMaximum(v);
}

void DoubleSliderWidget::blockSignals(bool block)
{
    spinBox_->blockSignals(block);
    slider_->blockSignals(block);
}

void DoubleSliderWidget::slotFinalValue()
{
    emit signalFinalValue();
}

void DoubleSliderWidget::slotIntermediateValue(int v)
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

void DoubleSliderWidget::create(DoubleSliderWidget *&outputWidget,
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
                                Layout layout)
{
    outputWidget = new DoubleSliderWidget();

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

    // Value Slider
    int stepInt = static_cast<int>(step);
    int minInt = static_cast<int>(min);
    int maxInt = static_cast<int>(max);
    int valueInt = static_cast<int>(value);

    outputWidget->slider_ = new QSlider;
    QSlider *slider = outputWidget->slider_;
    slider->setRange(minInt, maxInt);
    slider->setValue(valueInt);
    slider->setSingleStep(stepInt);
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
    spinBox->setRange(minInt, maxInt);
    spinBox->setValue(valueInt);
    spinBox->setSingleStep(stepInt);

    connect(spinBox,
            SIGNAL(valueChanged(int)),
            outputWidget,
            SLOT(slotIntermediateValue(int)));

    connect(spinBox,
            SIGNAL(editingFinished()),
            outputWidget,
            SLOT(slotFinalValue()));

    // Create widget layout.
    if (layout == LAYOUT_SLIDER_BESIDE_LABEL)
    {
        // Put everything on single line.
        QHBoxLayout *groupLayout = new QHBoxLayout;
        groupLayout->addWidget(label);
        groupLayout->addWidget(help);
        groupLayout->addWidget(slider);
        groupLayout->addWidget(spinBox);
        groupLayout->addWidget(units);

        outputWidget->setLayout(groupLayout);
    }
    else
    {
        // Create description layout.
        QHBoxLayout *descriptionLayout = new QHBoxLayout;
        descriptionLayout->addWidget(label);
        descriptionLayout->addWidget(help);
        descriptionLayout->addStretch();
        descriptionLayout->addWidget(units);

        // Create value layout.
        QHBoxLayout *valueLayout = new QHBoxLayout;
        valueLayout->addWidget(slider);
        valueLayout->addWidget(spinBox);

        // Group description and value layouts.
        QVBoxLayout *groupLayout = new QVBoxLayout;
        groupLayout->addLayout(descriptionLayout);
        groupLayout->addLayout(valueLayout);

        outputWidget->setLayout(groupLayout);
    }
}
