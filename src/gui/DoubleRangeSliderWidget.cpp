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

/** @file DoubleRangeSliderWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// Include 3rd party.
#include <ctkDoubleRangeSlider.h>

// Include local.
#define LOG_MODULE_NAME "DoubleRangeSliderWidget"
#include <Log.hpp>

DoubleRangeSliderWidget::DoubleRangeSliderWidget()
    : QWidget(),
      slider_(nullptr),
      minSpinBox_(nullptr),
      maxSpinBox_(nullptr),
      minimumValue_(0),
      maximumValue_(0)
{
}

void DoubleRangeSliderWidget::setMinimum(double min)
{
    LOG_DEBUG(<< "Set minimum <" << min << ">.");
    minSpinBox_->setMinimum(min);
    maxSpinBox_->setMinimum(min);
    slider_->setMinimum(min);
    minimumValue_ = min;
}

void DoubleRangeSliderWidget::setMaximum(double max)
{
    LOG_DEBUG(<< "Set maximum <" << max << ">.");
    minSpinBox_->setMaximum(max);
    maxSpinBox_->setMaximum(max);
    slider_->setMaximum(max);
    maximumValue_ = max;
}

void DoubleRangeSliderWidget::setMinimumValue(double value)
{
    LOG_DEBUG(<< "Set minimum value <" << value << ">.");
    minSpinBox_->setValue(value);
    slider_->setMinimumValue(value);
    minimumValue_ = value;
}

double DoubleRangeSliderWidget::minimumValue()
{
    return minimumValue_;
}

void DoubleRangeSliderWidget::setMaximumValue(double value)
{
    LOG_DEBUG(<< "Set maximum value <" << value << ">.");
    maxSpinBox_->setValue(value);
    slider_->setMaximumValue(value);
    maximumValue_ = value;
}

double DoubleRangeSliderWidget::maximumValue()
{
    return maximumValue_;
}

void DoubleRangeSliderWidget::blockSignals(bool block)
{
    minSpinBox_->blockSignals(block);
    maxSpinBox_->blockSignals(block);

    // slider_->blockSignals(block);
    if (block)
    {
        slider_->disconnectSlider();
    }
    else
    {
        slider_->connectSlider();
    }
}

void DoubleRangeSliderWidget::slotIntermediateMinimumValue(double v)
{
    LOG_DEBUG(<< "Update minimum value <" << v << ">.");

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

void DoubleRangeSliderWidget::slotIntermediateMaximumValue(double v)
{
    LOG_DEBUG(<< "Update maximum value <" << v << ">.");

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

void DoubleRangeSliderWidget::create(DoubleRangeSliderWidget *&outputWidget,
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
                                     double maxValue)
{
    LOG_DEBUG(<< "Create with parameter min <" << min << ">"
              << " max <" << max << ">"
              << " minValue <" << minValue << ">"
              << " maxValue <" << maxValue << ">.");

    outputWidget = new DoubleRangeSliderWidget();

    outputWidget->minimumValue_ = minValue;
    outputWidget->maximumValue_ = maxValue;

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

    // Description Layout.
    QHBoxLayout *descriptionLayout = new QHBoxLayout;
    descriptionLayout->addWidget(label);
    descriptionLayout->addWidget(help);
    descriptionLayout->addStretch();
    descriptionLayout->addWidget(units);

    // Value Slider.
    outputWidget->slider_ = new ctkDoubleRangeSlider;
    ctkDoubleRangeSlider *slider = outputWidget->slider_;
    slider->setRange(min, max);
    slider->setValues(minValue, maxValue);
    slider->setSingleStep(step);
    slider->setOrientation(Qt::Horizontal);

    connect(slider,
            SIGNAL(minimumPositionChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMinimumValue(double)));

    if (memberIntermediateMinimumValue)
    {
        connect(outputWidget,
                SIGNAL(signalIntermediateMinimumValue()),
                receiver,
                memberIntermediateMinimumValue);
    }

    connect(slider,
            SIGNAL(maximumPositionChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMaximumValue(double)));

    if (memberIntermediateMaximumValue)
    {
        connect(outputWidget,
                SIGNAL(signalIntermediateMaximumValue()),
                receiver,
                memberIntermediateMaximumValue);
    }

    // Value SpinBox.
    outputWidget->minSpinBox_ = new QDoubleSpinBox;
    QDoubleSpinBox *minSpinBox = outputWidget->minSpinBox_;
    minSpinBox->setRange(min, max);
    minSpinBox->setValue(minValue);
    minSpinBox->setSingleStep(step);

    connect(minSpinBox,
            SIGNAL(valueChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMinimumValue(double)));

    outputWidget->maxSpinBox_ = new QDoubleSpinBox;
    QDoubleSpinBox *maxSpinBox = outputWidget->maxSpinBox_;
    maxSpinBox->setRange(min, max);
    maxSpinBox->setValue(maxValue);
    maxSpinBox->setSingleStep(step);

    connect(maxSpinBox,
            SIGNAL(valueChanged(double)),
            outputWidget,
            SLOT(slotIntermediateMaximumValue(double)));

    // Value Layout.
    QHBoxLayout *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(new QLabel("Min"));
    valueLayout->addWidget(minSpinBox);
    valueLayout->addWidget(new QLabel("Max"));
    valueLayout->addWidget(maxSpinBox);

    // Group Description and Value.
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupLayout->addLayout(descriptionLayout);
    groupLayout->addWidget(slider);
    groupLayout->addLayout(valueLayout);

    outputWidget->setLayout(groupLayout);
}
