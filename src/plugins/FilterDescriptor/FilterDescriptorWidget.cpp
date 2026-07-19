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

/** @file FilterDescriptorWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <DoubleRangeSliderWidget.hpp>
#include <FilterDescriptorWidget.hpp>
#include <HBoxLayout.hpp>
#include <PushButton.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterDescriptorWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterDescriptorWidget::FilterDescriptorWidget(Application *app)
    : Widget(app),
      app_(app)
{
    LOG_DEBUG(<< "Start creating descriptor filter widget.");

    // Input widgets.
    DoubleRangeSliderWidget::create(
        descriptorInput_,
        [this](double val) { slotMinimumValueChanged(val); },
        [this](double val) { slotMaximumValueChanged(val); },
        [this]() { slotFinalValue(); },
        tr("Descriptor"),
        tr("Min-max descriptor range filter"),
        tr("%"),
        1,
        0,
        100,
        0,
        100);

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(descriptorInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect(
        [this](void *sender, const std::set<Editor::Type> &target)
        { slotUpdate(sender, target); });

    slotUpdate(nullptr, std::set<Editor::Type>());

    LOG_DEBUG(<< "Finished creating descriptor filter widget.");
}

void FilterDescriptorWidget::slotUpdate(void *sender,
                                        const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_DESCRIPTOR) ||
        target.count(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input descriptor filter.");
        setDescriptor(app_->editor().descriptorFilter());
    }
}

void FilterDescriptorWidget::setDescriptor(const Range<double> &range)
{
    LOG_DEBUG(<< "Set descriptor filter range <" << range << ">.");

    descriptorRange_ = range;
    double min = descriptorRange_.minimum() * 100.0;
    double max = descriptorRange_.maximum() * 100.0;

    descriptorInput_->blockSignals(true);
    descriptorInput_->setMinimum(min);
    descriptorInput_->setMaximum(max);
    descriptorInput_->setMinimumValue(min);
    descriptorInput_->setMaximumValue(max);
    descriptorInput_->blockSignals(false);
}

void FilterDescriptorWidget::filterChanged(bool final)
{
    LOG_DEBUG(<< "Descriptor filer changed.");

    app_->suspendThreads();
    app_->editor().setDescriptorFilter(descriptorRange_);
    app_->updateFilter(this, final);
}

void FilterDescriptorWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set descriptor filer enabled <" << toString(b) << ">.");

    descriptorRange_.setEnabled(b);
    filterChanged(true);
}

void FilterDescriptorWidget::slotMinimumValueChanged(double val)
{
    LOG_DEBUG(<< "Minimum value changed.");

    descriptorRange_.setMinimumValue(descriptorInput_->minimumValue() * 0.01);
    filterChanged(false);
}

void FilterDescriptorWidget::slotMaximumValueChanged(double val)
{
    LOG_DEBUG(<< "Maximum value changed.");

    descriptorRange_.setMaximumValue(descriptorInput_->maximumValue() * 0.01);
    filterChanged(false);
}

void FilterDescriptorWidget::slotFinalValue()
{
    filterChanged(true);
}
