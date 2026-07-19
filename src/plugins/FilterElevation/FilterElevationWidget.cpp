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

/** @file FilterElevationWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <DoubleRangeSliderWidget.hpp>
#include <FilterElevationWidget.hpp>
#include <HBoxLayout.hpp>
#include <PushButton.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "FilterElevationWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterElevationWidget::FilterElevationWidget(Application *app)
    : Widget(app),
      app_(app)
{
    LOG_DEBUG(<< "Start creating elevation filter widget.");

    // Input widgets.
    DoubleRangeSliderWidget::create(
        elevationInput_,
        [this](double val) { slotMinimumValueChanged(val); },
        [this](double val) { slotMaximumValueChanged(val); },
        [this]() { slotFinalValue(); },
        tr("Elevation"),
        tr("Min-max elevation range filter"),
        tr("m"),
        0.01,
        0,
        100,
        0,
        100);

    // Layout.
    VBoxLayout *mainLayout = new VBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(elevationInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    app_->signalUpdate.connect(
        [this](void *sender, const std::set<Editor::Type> &target)
        { slotUpdate(sender, target); });

    slotUpdate(nullptr, std::set<Editor::Type>());

    LOG_DEBUG(<< "Finished creating elevation filter widget.");
}

void FilterElevationWidget::slotUpdate(void *sender,
                                       const std::set<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.count(Editor::TYPE_ELEVATION) ||
        target.count(Editor::TYPE_SETTINGS) ||
        target.count(Editor::TYPE_DATA_SET))
    {
        LOG_DEBUG_UPDATE(<< "Input elevation filter.");
        setElevation(app_->editor().elevationFilter());
    }
}

void FilterElevationWidget::setElevation(const Range<double> &range)
{
    LOG_DEBUG(<< "Set elevation filter range <" << range << ">.");

    elevationRange_ = range;

    double ppm = app_->editor().settings().unitsSettings().pointsPerMeter()[0];

    double min = elevationRange_.minimum() / ppm;
    double max = elevationRange_.maximum() / ppm;

    double minValue = elevationRange_.minimumValue() / ppm;
    double maxValue = elevationRange_.maximumValue() / ppm;

    elevationInput_->blockSignals(true);
    elevationInput_->setMinimum(min);
    elevationInput_->setMaximum(max);
    elevationInput_->setMinimumValue(minValue);
    elevationInput_->setMaximumValue(maxValue);
    elevationInput_->blockSignals(false);
}

void FilterElevationWidget::filterChanged(bool final)
{
    LOG_DEBUG(<< "Elevation filer changed.");

    app_->suspendThreads();
    app_->editor().setElevationFilter(elevationRange_);
    app_->updateFilter(this, final);
}

void FilterElevationWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set elevation filer enabled <" << toString(b) << ">.");

    elevationRange_.setEnabled(b);
    filterChanged(true);
}

void FilterElevationWidget::slotMinimumValueChanged(double val)
{
    LOG_DEBUG(<< "Minimum value changed.");

    double ppm = app_->editor().settings().unitsSettings().pointsPerMeter()[0];

    elevationRange_.setMinimumValue(elevationInput_->minimumValue() * ppm);
    filterChanged(false);
}

void FilterElevationWidget::slotMaximumValueChanged(double val)
{
    LOG_DEBUG(<< "Maximum value changed.");

    double ppm = app_->editor().settings().unitsSettings().pointsPerMeter()[0];

    elevationRange_.setMaximumValue(elevationInput_->maximumValue() * ppm);
    filterChanged(false);
}

void FilterElevationWidget::slotFinalValue()
{
    filterChanged(true);
}
