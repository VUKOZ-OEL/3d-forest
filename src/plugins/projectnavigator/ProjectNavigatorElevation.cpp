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

/** @file ProjectNavigatorElevation.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorElevation.hpp>
#include <RangeSliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/projectnavigator/", name))

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("ProjectNavigatorElevation", msg)

ProjectNavigatorElevation::ProjectNavigatorElevation(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    LOG_DEBUG_LOCAL("");

    // Input widgets
    RangeSliderWidget::create(rangeInput_,
                              this,
                              SLOT(slotRangeIntermediateMinimumValue(int)),
                              SLOT(slotRangeIntermediateMaximumValue(int)),
                              tr("Range"),
                              tr("Min-max elevation range filter"),
                              tr("pt"),
                              1,
                              0,
                              100,
                              0,
                              100);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(rangeInput_);
    mainLayout->addStretch();
    setLayout(mainLayout);

    // Data
    connect(mainWindow_, SIGNAL(signalUpdate()), this, SLOT(slotUpdate()));
}

void ProjectNavigatorElevation::slotRangeIntermediateMinimumValue(int v)
{
    LOG_DEBUG_LOCAL("minimumValue <" << rangeInput_->minimumValue() << "> v <"
                                     << v << ">");

    elevationRange_.setMinimumValue(v);

    filterChanged();
}

void ProjectNavigatorElevation::slotRangeIntermediateMaximumValue(int v)
{
    LOG_DEBUG_LOCAL("maximumValue <" << rangeInput_->maximumValue() << "> v <"
                                     << v << ">");

    elevationRange_.setMaximumValue(v);

    filterChanged();
}

void ProjectNavigatorElevation::filterChanged()
{
    LOG_DEBUG_LOCAL("");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setElevationRange(elevationRange_);
    mainWindow_->updateFilter();
}

void ProjectNavigatorElevation::slotUpdate()
{
    elevationRange_ = mainWindow_->editor().elevationRange();
    LOG_DEBUG_LOCAL("elevationRange <" << elevationRange_ << ">");

    rangeInput_->blockSignals(true);
    rangeInput_->setMinimum(elevationRange_.minimum());
    rangeInput_->setMaximum(elevationRange_.maximum());
    rangeInput_->setMinimumValue(elevationRange_.minimumValue());
    rangeInput_->setMaximumValue(elevationRange_.maximumValue());
    rangeInput_->blockSignals(false);
}
