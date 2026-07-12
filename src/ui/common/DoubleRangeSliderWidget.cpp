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
#include <DoubleRangeSlider.hpp>
#include <Application.hpp>
#include <Slider.hpp>
#include <DoubleSpinBox.hpp>
#include <ComboBox.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "DoubleRangeSliderWidget"
#include <Log.hpp>

DoubleRangeSliderWidget::DoubleRangeSliderWidget()
    : Widget(),
      slider_(nullptr),
      minSpinBox_(nullptr),
      maxSpinBox_(nullptr)
{
}

DoubleRangeSliderWidget::~DoubleRangeSliderWidget()
{
}

double DoubleRangeSliderWidget::minimum() const
{
    return minSpinBox_->minimum();
}

void DoubleRangeSliderWidget::setMinimum(double min)
{
    LOG_DEBUG(<< "Set minimum <" << min << ">.");
    minSpinBox_->setMinimum(min);
    slider_->setMinimum(min);
    maxSpinBox_->setMinimum(min);
}

double DoubleRangeSliderWidget::maximum() const
{
    return minSpinBox_->maximum();
}

void DoubleRangeSliderWidget::setMaximum(double max)
{
    LOG_DEBUG(<< "Set maximum <" << max << ">.");
    minSpinBox_->setMaximum(max);
    slider_->setMaximum(max);
    maxSpinBox_->setMaximum(max);
}

// void DoubleRangeSliderWidget::blockSignals(bool block)
// {
//    minSpinBox_->blockSignals(block);
//    maxSpinBox_->blockSignals(block);
//
//    // slider_->blockSignals(block);
//    if (block)
//    {
//        slider_->disconnectSlider();
//    }
//    else
//    {
//        slider_->connectSlider();
//    }
// }

double DoubleRangeSliderWidget::minimumValue() const
{
    return minSpinBox_->value();
}

void DoubleRangeSliderWidget::setMinimumValue(double value, bool notify)
{
    LOG_DEBUG(<< "Set minimum value <" << value << ">.");
    minSpinBox_->setValue(value);
    slider_->setMinimumValue(value);
}

double DoubleRangeSliderWidget::maximumValue() const
{
    return maxSpinBox_->value();
}

void DoubleRangeSliderWidget::setMaximumValue(double value, bool notify)
{
    LOG_DEBUG(<< "Set maximum value <" << value << ">.");
    maxSpinBox_->setValue(value);
    slider_->setMaximumValue(value);
}

void DoubleRangeSliderWidget::slotMinimumValueChangedSlider(double v)
{
    LOG_DEBUG(<< "Update minimum value <" << v << ">.");

    minSpinBox_->blockSignals(true);
    minSpinBox_->setValue(v);
    minSpinBox_->blockSignals(false);

    minimumValueChanged(v);
}

void DoubleRangeSliderWidget::slotMaximumValueChangedSlider(double v)
{
    LOG_DEBUG(<< "Update maximum value <" << v << ">.");

    maxSpinBox_->blockSignals(true);
    maxSpinBox_->setValue(v);
    maxSpinBox_->blockSignals(false);

    maximumValueChanged(v);
}

void DoubleRangeSliderWidget::slotMinimumValueChangedSpinBox(double v)
{
    LOG_DEBUG(<< "Update minimum value <" << v << ">.");

    slider_->blockSignals(true);
    slider_->setMinimumValue(v);
    slider_->blockSignals(false);

    minimumValueChanged(v);
}

void DoubleRangeSliderWidget::slotMaximumValueChangedSpinBox(double v)
{
    LOG_DEBUG(<< "Update maximum value <" << v << ">.");

    slider_->blockSignals(true);
    slider_->setMaximumValue(v);
    slider_->blockSignals(false);

    maximumValueChanged(v);
}

void DoubleRangeSliderWidget::slotFinalValueSlider()
{
    LOG_DEBUG(<< "Final value slider.");
    finalValue();
}

void DoubleRangeSliderWidget::slotFinalValueSpinBox()
{
    LOG_DEBUG(<< "Final value spinBox.");
    finalValue();
}

void DoubleRangeSliderWidget::create(DoubleRangeSliderWidget *&outputWidget,
                       std::function<void(double)> minValueChangedCallback,
                       std::function<void(double)> maxValueChangedCallback,
                       std::function<void()> finalValueCallback,
                       const std::string &text,
                       const std::string &toolTip,
                       const std::string &unitsList,
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

    // Description Name.
    Label *label = new Label(text);

    // Description Tool Tip.
    Label *help = new Label;
    help->setToolTip(toolTip);
    ThemeIcon helpIcon(":/gui/", "question");
    help->setPixmap(helpIcon.pixmap(Application::ICON_SIZE_TEXT));

    // Description Units.
    ComboBox *units = new ComboBox;
    units->addItem(unitsList);

    // Description Layout.
    HBoxLayout *descriptionLayout = new HBoxLayout;
    descriptionLayout->addWidget(label);
    descriptionLayout->addWidget(help);
    descriptionLayout->addStretch();
    descriptionLayout->addWidget(units);

    // Value Slider.
    outputWidget->slider_ = new DoubleRangeSlider;
    DoubleRangeSlider *slider = outputWidget->slider_;
    slider->setRange(min, max);
    slider->setValues(minValue, maxValue);
    slider->setSingleStep(step);
    slider->setOrientation(Ui::Horizontal);

    slider->minimumValueChanged.connect([outputWidget](int value)
    {
        outputWidget->slotMinimumValueChangedSlider(value);
    });

    slider->maximumValueChanged.connect([outputWidget](int value)
    {
        outputWidget->slotMaximumValueChangedSlider(value);
    });

    slider->sliderReleased.connect([outputWidget]()
    {
        outputWidget->slotFinalValueSlider();
    });

    // Value SpinBox.
    outputWidget->minSpinBox_ = new DoubleSpinBox;
    DoubleSpinBox *minSpinBox = outputWidget->minSpinBox_;
    minSpinBox->setRange(min, max);
    minSpinBox->setValue(minValue);
    minSpinBox->setSingleStep(step);

    minSpinBox->valueChanged.connect([outputWidget](double value)
    {
        outputWidget->slotMinimumValueChangedSpinBox(value);
    });

    minSpinBox->editingFinished.connect([outputWidget]()
    {
        outputWidget->slotFinalValueSpinBox();
    });

    outputWidget->maxSpinBox_ = new DoubleSpinBox;
    DoubleSpinBox *maxSpinBox = outputWidget->maxSpinBox_;
    maxSpinBox->setRange(min, max);
    maxSpinBox->setValue(maxValue);
    maxSpinBox->setSingleStep(step);

    maxSpinBox->valueChanged.connect([outputWidget](double value)
    {
        outputWidget->slotMaximumValueChangedSpinBox(value);
    });

    maxSpinBox->editingFinished.connect([outputWidget]()
    {
        outputWidget->slotFinalValueSpinBox();
    });

    // Value Layout.
    HBoxLayout *valueLayout = new HBoxLayout;
    valueLayout->addWidget(new Label("Min"));
    valueLayout->addWidget(minSpinBox);
    valueLayout->addWidget(new Label("Max"));
    valueLayout->addWidget(maxSpinBox);

    // Group Description and Value.
    VBoxLayout *groupLayout = new VBoxLayout;
    groupLayout->addLayout(descriptionLayout);
    groupLayout->addWidget(slider);
    groupLayout->addLayout(valueLayout);

    outputWidget->setLayout(groupLayout);

    // Connect.
    if (minValueChangedCallback)
    {
        outputWidget->minimumValueChanged.connect(
            std::move(minValueChangedCallback));
    }

    if (maxValueChangedCallback)
    {
        outputWidget->maximumValueChanged.connect(
            std::move(maxValueChangedCallback));
    }

    if (finalValueCallback)
    {
        outputWidget->finalValue.connect(
            std::move(finalValueCallback));
    }
}
