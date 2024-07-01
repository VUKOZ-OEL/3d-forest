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

// Include 3D Forest.
#include <DoubleSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "DoubleSliderWidget"
#include <Log.hpp>

DoubleSliderWidget::DoubleSliderWidget()
    : QWidget(),
      slider_(nullptr),
      spinBox_(nullptr),
      minimumValue_(0),
      maximumValue_(0)
{
}

double DoubleSliderWidget::value() const
{
    return spinBox_->value();
}

double DoubleSliderWidget::minimum() const
{
    return maximumValue_;
}

double DoubleSliderWidget::maximum() const
{
    return maximumValue_;
}

void DoubleSliderWidget::setValue(double value)
{
    spinBox_->setValue(value);

    int valueInt = static_cast<int>(
        ((value - minimumValue_) / (maximumValue_ - minimumValue_)) * 1000);
    slider_->setValue(valueInt);
}

void DoubleSliderWidget::setMinimum(double min)
{
    spinBox_->setMinimum(min);
    slider_->setMinimum(0);
    minimumValue_ = min;
}

void DoubleSliderWidget::setMaximum(double max)
{
    spinBox_->setMaximum(max);
    slider_->setMaximum(1000);
    maximumValue_ = max;
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

    double value = minimumValue_ + (static_cast<double>(v) * 0.001 *
                                    (maximumValue_ - minimumValue_));

    if (obj == slider_)
    {
        spinBox_->blockSignals(true);
        spinBox_->setValue(value);
        spinBox_->blockSignals(false);
    }

    emit signalIntermediateValue(value);
}

void DoubleSliderWidget::slotIntermediateValue(double v)
{
    QObject *obj = sender();

    int valueInt = static_cast<int>(
        ((v - minimumValue_) / (maximumValue_ - minimumValue_)) * 1000);

    if (obj == spinBox_)
    {
        slider_->blockSignals(true);
        slider_->setValue(valueInt);
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

    outputWidget->minimumValue_ = min;
    outputWidget->maximumValue_ = max;

    // Description Name.
    QLabel *label = new QLabel(text);

    // Description Tool Tip.
    QLabel *help = new QLabel;
    help->setToolTip(toolTip);
    ThemeIcon helpIcon(":/gui/", "question");
    help->setPixmap(helpIcon.pixmap(MainWindow::ICON_SIZE_TEXT));

    // Description Units.
    QComboBox *units = new QComboBox;
    units->addItem(unitsList);

    // Value Slider.
    int stepInt = 1;
    int minInt = 0;
    int maxInt = 1000;
    int valueInt = static_cast<int>(((value - min) / (max - min)) * 1000);

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
                SIGNAL(signalIntermediateValue(double)),
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

    // Value SpinBox.
    outputWidget->spinBox_ = new QDoubleSpinBox;
    QDoubleSpinBox *spinBox = outputWidget->spinBox_;
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSingleStep(step);

    connect(spinBox,
            SIGNAL(valueChanged(double)),
            outputWidget,
            SLOT(slotIntermediateValue(double)));

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
