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
#include <DoubleRangeSliderWidget.hpp>
#include <FilterElevationWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterElevationWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterElevationWidget::FilterElevationWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating elevation filter widget.");

    // Input widgets.
    DoubleRangeSliderWidget::create(elevationInput_,
                                    this,
                                    SLOT(slotRangeIntermediateMinimumValue()),
                                    SLOT(slotRangeIntermediateMaximumValue()),
                                    tr("Elevation"),
                                    tr("Min-max elevation range filter"),
                                    tr("m"),
                                    0.01,
                                    0,
                                    100,
                                    0,
                                    100);

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(elevationInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating elevation filter widget.");
}

void FilterElevationWidget::slotUpdate(void *sender,
                                       const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_ELEVATION) ||
        target.contains(Editor::TYPE_SETTINGS) ||
        target.contains(Editor::TYPE_DATA_SET))
    {
        LOG_DEBUG_UPDATE(<< "Input elevation filter.");
        setElevation(mainWindow_->editor().elevationFilter());
    }
}

void FilterElevationWidget::setElevation(const Range<double> &range)
{
    LOG_DEBUG(<< "Set elevation filter range <" << range << ">.");

    elevationRange_ = range;

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];
    double min = elevationRange_.minimum() / ppm;
    double max = elevationRange_.maximum() / ppm;

    elevationInput_->blockSignals(true);
    elevationInput_->setMinimum(min);
    elevationInput_->setMaximum(max);
    elevationInput_->setMinimumValue(min);
    elevationInput_->setMaximumValue(max);
    elevationInput_->blockSignals(false);
}

void FilterElevationWidget::filterChanged()
{
    LOG_DEBUG(<< "Elevation filer changed.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setElevationFilter(elevationRange_);
    mainWindow_->updateFilter();
}

void FilterElevationWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set elevation filer enabled <" << toString(b) << ">.");

    elevationRange_.setEnabled(b);
    filterChanged();
}

void FilterElevationWidget::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Minimum value changed.");

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    elevationRange_.setMinimumValue(elevationInput_->minimumValue() * ppm);
    filterChanged();
}

void FilterElevationWidget::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Maximum value changed.");

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    elevationRange_.setMaximumValue(elevationInput_->maximumValue() * ppm);
    filterChanged();
}
