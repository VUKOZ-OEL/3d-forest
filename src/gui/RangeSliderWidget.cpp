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

/** @file RangeSliderWidget.cpp */

#include <MainWindow.hpp>
#include <RangeSliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <ctkRangeSlider.h>

#define LOG_MODULE_NAME "RangeSliderWidget"
#include <Log.hpp>

RangeSliderWidget::RangeSliderWidget()
    : QWidget(),
      slider_(nullptr),
      minSpinBox_(nullptr),
      maxSpinBox_(nullptr),
      minimumValue_(0),
      maximumValue_(0)
{
}

void RangeSliderWidget::setMinimum(int min)
{
    LOG_DEBUG(<< "Called with parameter min <" << min << ">.");
    minSpinBox_->setMinimum(min);
    maxSpinBox_->setMinimum(min);
    slider_->setMinimum(min);
    minimumValue_ = min;
}

void RangeSliderWidget::setMaximum(int max)
{
    LOG_DEBUG(<< "Called with parameter min <" << max << ">.");
    minSpinBox_->setMaximum(max);
    maxSpinBox_->setMaximum(max);
    slider_->setMaximum(max);
    maximumValue_ = max;
}

void RangeSliderWidget::setMinimumValue(int value)
{
    LOG_DEBUG(<< "Called with parameter value <" << value << ">.");
    minSpinBox_->setValue(value);
    slider_->setMinimumValue(value);
    minimumValue_ = value;
}

int RangeSliderWidget::minimumValue()
{
    return minimumValue_;
}

void RangeSliderWidget::setMaximumValue(int value)
{
    LOG_DEBUG(<< "Called with parameter value <" << value << ">.");
    maxSpinBox_->setValue(value);
    slider_->setMaximumValue(value);
    maximumValue_ = value;
}

int RangeSliderWidget::maximumValue()
{
    return maximumValue_;
}

void RangeSliderWidget::blockSignals(bool block)
{
    minSpinBox_->blockSignals(block);
    maxSpinBox_->blockSignals(block);
    slider_->blockSignals(block);
}

void RangeSliderWidget::slotIntermediateMinimumValue(int v)
{
    LOG_DEBUG(<< "Called with parameter value <" << v << ">.");

    QObject *obj = sender();

    if (obj == slider_)
    {
        minSpinBox_->blockSignals(true);
        minSpinBox_->setValue(v);
        minSpinBox_->blockSignals(false);
    }
    else if (obj == minSpinBox_)
    {
        slider_->blockSignals(true);
        slider_->setMinimumValue(v);
        slider_->blockSignals(false);
    }

    minimumValue_ = v;

    emit signalIntermediateMinimumValue();
}

void RangeSliderWidget::slotIntermediateMaximumValue(int v)
{
    LOG_DEBUG(<< "Called with parameter value <" << v << ">.");

    QObject *obj = sender();

    if (obj == slider_)
    {
        maxSpinBox_->blockSignals(true);
        maxSpinBox_->setValue(v);
        maxSpinBox_->blockSignals(false);
    }
    else if (obj == maxSpinBox_)
    {
        slider_->blockSignals(true);
        slider_->setMaximumValue(v);
        slider_->blockSignals(false);
    }

    maximumValue_ = v;

    emit signalIntermediateMaximumValue();
}

void RangeSliderWidget::create(RangeSliderWidget *&outputWidget,
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
                               int maxValue)
{
    LOG_DEBUG(<< "Called with parameter min <" << min << ">"
              << " max <" << max << ">"
              << " minValue <" << minValue << ">"
              << " maxValue <" << maxValue << ">.");

    outputWidget = new RangeSliderWidget();

    outputWidget->minimumValue_ = minValue;
    outputWidget->maximumValue_ = maxValue;

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
    outputWidget->slider_ = new ctkRangeSlider;
    ctkRangeSlider *slider = outputWidget->slider_;
    slider->setRange(min, max);
    slider->setValues(minValue, maxValue);
    slider->setSingleStep(step);
    slider->setOrientation(Qt::Horizontal);

    connect(slider,
            SIGNAL(minimumPositionChanged(int)),
            outputWidget,
            SLOT(slotIntermediateMinimumValue(int)));

    if (memberIntermediateMinimumValue)
    {
        connect(outputWidget,
                SIGNAL(signalIntermediateMinimumValue()),
                receiver,
                memberIntermediateMinimumValue);
    }

    connect(slider,
            SIGNAL(maximumPositionChanged(int)),
            outputWidget,
            SLOT(slotIntermediateMaximumValue(int)));

    if (memberIntermediateMaximumValue)
    {
        connect(outputWidget,
                SIGNAL(signalIntermediateMaximumValue()),
                receiver,
                memberIntermediateMaximumValue);
    }

    // Value SpinBox
    outputWidget->minSpinBox_ = new QSpinBox;
    QSpinBox *minSpinBox = outputWidget->minSpinBox_;
    minSpinBox->setRange(min, max);
    minSpinBox->setValue(minValue);
    minSpinBox->setSingleStep(step);

    connect(minSpinBox,
            SIGNAL(valueChanged(int)),
            outputWidget,
            SLOT(slotIntermediateMinimumValue(int)));

    outputWidget->maxSpinBox_ = new QSpinBox;
    QSpinBox *maxSpinBox = outputWidget->maxSpinBox_;
    maxSpinBox->setRange(min, max);
    maxSpinBox->setValue(maxValue);
    maxSpinBox->setSingleStep(step);

    connect(maxSpinBox,
            SIGNAL(valueChanged(int)),
            outputWidget,
            SLOT(slotIntermediateMaximumValue(int)));

    // Value Layout
    QHBoxLayout *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(new QLabel("Min"));
    valueLayout->addWidget(minSpinBox);
    valueLayout->addWidget(new QLabel("Max"));
    valueLayout->addWidget(maxSpinBox);

    // Group Description and Value
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupLayout->addLayout(descriptionLayout);
    groupLayout->addWidget(slider);
    groupLayout->addLayout(valueLayout);

    outputWidget->setLayout(groupLayout);
}
