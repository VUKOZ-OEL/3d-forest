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

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <ComboBox.hpp>
#include <DoubleSliderWidget.hpp>
#include <DoubleSpinBox.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <Slider.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "DoubleSliderWidget"
#include <Log.hpp>

DoubleSliderWidget::DoubleSliderWidget()
    : Widget(),
      slider_(nullptr),
      spinBox_(nullptr),
      targetProduct_(0)
{
}

DoubleSliderWidget::~DoubleSliderWidget()
{
}

double DoubleSliderWidget::value() const
{
    return spinBox_->value();
}

void DoubleSliderWidget::setValue(double value, bool notify)
{
    double min = minimum();
    double max = maximum();
    clamp(value, min, max);

    spinBox_->setValue(value);

    int valueInt = 0;
    if (max - min > 0.0)
    {
        valueInt = static_cast<int>(((value - min) / (max - min)) * 1000);
    }
    slider_->setValue(valueInt);
}

double DoubleSliderWidget::minimum() const
{
    return spinBox_->minimum();
}

void DoubleSliderWidget::setMinimum(double min)
{
    spinBox_->setMinimum(min);
    slider_->setMinimum(0);
}

double DoubleSliderWidget::maximum() const
{
    return spinBox_->maximum();
}

void DoubleSliderWidget::setMaximum(double max)
{
    spinBox_->setMaximum(max);
    slider_->setMaximum(1000);
}

void DoubleSliderWidget::setTargetProduct(double value)
{
    targetProduct_ = value;
}

// void DoubleSliderWidget::blockSignals(bool block)
// {
//     spinBox_->blockSignals(block);
//     slider_->blockSignals(block);
// }

void DoubleSliderWidget::slotValueChangedSlider(int v)
{
    double min = minimum();
    double max = maximum();
    double value = min + (static_cast<double>(v) * 0.001 * (max - min));

    value = snapToReciprocal(value);

    spinBox_->blockSignals(true);
    spinBox_->setValue(value);
    spinBox_->blockSignals(false);

    valueChanged(value);
}

void DoubleSliderWidget::slotValueChangedSpinBox(double v)
{
    v = snapToReciprocal(v);

    double min = minimum();
    double max = maximum();
    clamp(v, min, max);

    int valueInt = 0;
    if (max - min > 0.0)
    {
        valueInt = static_cast<int>((v - min) / (max - min)) * 1000;
    }

    slider_->blockSignals(true);
    slider_->setValue(valueInt);
    slider_->blockSignals(false);

    valueChanged(v);
}

void DoubleSliderWidget::slotFinalValueSlider()
{
    finalValue();
}

void DoubleSliderWidget::slotFinalValueSpinBox()
{
    finalValue();
}

double DoubleSliderWidget::snapToReciprocal(double value) const
{
    double result = value;

    if (targetProduct_ > 0.0 && value > 0.0)
    {
        double x = std::round(targetProduct_ / value);
        if (x > 0.0)
        {
            result = targetProduct_ / x;
        }
    }

    return result;
}

void DoubleSliderWidget::create(
    DoubleSliderWidget *&outputWidget,
    std::function<void(double)> valueChangedCallback,
    std::function<void()> finalValueCallback,
    const std::string &text,
    const std::string &toolTip,
    const std::string &unitsList,
    double step,
    double min,
    double max,
    double value,
    Layout layout)
{
    outputWidget = new DoubleSliderWidget();

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

    // Value Slider.
    int stepInt = 1;
    int minInt = 0;
    int maxInt = 1000;
    int valueInt = static_cast<int>(((value - min) / (max - min)) * 1000);

    outputWidget->slider_ = new Slider;
    Slider *slider = outputWidget->slider_;
    slider->setRange(minInt, maxInt);
    slider->setValue(valueInt);
    slider->setSingleStep(stepInt);
    slider->setOrientation(Ui::Horizontal);

    slider->valueChanged.connect(
        [outputWidget](int value)
        { outputWidget->slotValueChangedSlider(value); });

    slider->sliderReleased.connect([outputWidget]()
                                   { outputWidget->slotFinalValueSlider(); });

    // Value SpinBox.
    outputWidget->spinBox_ = new DoubleSpinBox;
    DoubleSpinBox *spinBox = outputWidget->spinBox_;
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSingleStep(step);

    spinBox->valueChanged.connect(
        [outputWidget](double value)
        { outputWidget->slotValueChangedSpinBox(value); });

    spinBox->editingFinished.connect(
        [outputWidget]() { outputWidget->slotFinalValueSpinBox(); });

    // Create widget layout.
    if (layout == LAYOUT_SLIDER_BESIDE_LABEL)
    {
        // Put everything on single line.
        HBoxLayout *groupLayout = new HBoxLayout;
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
        HBoxLayout *descriptionLayout = new HBoxLayout;
        descriptionLayout->addWidget(label);
        descriptionLayout->addWidget(help);
        descriptionLayout->addStretch();
        descriptionLayout->addWidget(units);

        // Create value layout.
        HBoxLayout *valueLayout = new HBoxLayout;
        valueLayout->addWidget(slider);
        valueLayout->addWidget(spinBox);

        // Group description and value layouts.
        VBoxLayout *groupLayout = new VBoxLayout;
        groupLayout->addLayout(descriptionLayout);
        groupLayout->addLayout(valueLayout);

        outputWidget->setLayout(groupLayout);
    }

    // Connect.
    if (valueChangedCallback)
    {
        outputWidget->valueChanged.connect(std::move(valueChangedCallback));
    }

    if (finalValueCallback)
    {
        outputWidget->finalValue.connect(std::move(finalValueCallback));
    }
}
