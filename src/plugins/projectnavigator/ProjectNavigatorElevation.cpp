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

#define MODULE_NAME "ProjectNavigatorElevation"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ProjectNavigatorElevation::ProjectNavigatorElevation(MainWindow *mainWindow)
    : QWidget(),
      mainWindow_(mainWindow)
{
    LOG_DEBUG_LOCAL("");

    // Input widgets
    RangeSliderWidget::create(rangeInput_,
                              this,
                              SLOT(slotRangeIntermediateMinimumValue()),
                              SLOT(slotRangeIntermediateMaximumValue()),
                              tr("Elevation"),
                              tr("Min-max elevation range filter"),
                              tr("pt"),
                              1,
                              0,
                              100,
                              0,
                              100);

    RangeSliderWidget::create(descriptorInput_,
                              this,
                              SLOT(slotDescriptorIntermediateMinimumValue()),
                              SLOT(slotDescriptorIntermediateMaximumValue()),
                              tr("Descriptor"),
                              tr("Min-max descriptor range filter"),
                              tr("pt"),
                              1,
                              0,
                              100,
                              0,
                              100);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(rangeInput_);
    mainLayout->addWidget(descriptorInput_);
    mainLayout->addStretch();
    setLayout(mainLayout);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(const QSet<Editor::Type> &)));
}

void ProjectNavigatorElevation::slotUpdate(const QSet<Editor::Type> &target)
{
    LOG_DEBUG_LOCAL("");

    if (target.empty() || target.contains(Editor::TYPE_ELEVATION))
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

    if (target.empty() || target.contains(Editor::TYPE_DESCRIPTOR))
    {
        descriptorRange_ = mainWindow_->editor().descriptorRange();
        LOG_DEBUG_LOCAL("descriptorRange <" << descriptorRange_ << ">");

        descriptorInput_->blockSignals(true);
        descriptorInput_->setMinimum(descriptorRange_.minimum() * 255.0);
        descriptorInput_->setMaximum(descriptorRange_.maximum() * 255.0);
        descriptorInput_->setMinimumValue(descriptorRange_.minimumValue() *
                                          255.0);
        descriptorInput_->setMaximumValue(descriptorRange_.maximumValue() *
                                          255.0);
        descriptorInput_->blockSignals(false);
    }
}

void ProjectNavigatorElevation::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG_LOCAL("minimumValue <" << rangeInput_->minimumValue() << ">");
    elevationRange_.setMinimumValue(rangeInput_->minimumValue());
    elevationInputChanged();
}

void ProjectNavigatorElevation::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG_LOCAL("maximumValue <" << rangeInput_->maximumValue() << ">");
    elevationRange_.setMaximumValue(rangeInput_->maximumValue());
    elevationInputChanged();
}

void ProjectNavigatorElevation::elevationInputChanged()
{
    LOG_DEBUG_LOCAL("");
    mainWindow_->suspendThreads();
    mainWindow_->editor().setElevationRange(elevationRange_);
    mainWindow_->updateFilter();
}

void ProjectNavigatorElevation::slotDescriptorIntermediateMinimumValue()
{
    LOG_DEBUG_LOCAL("minimumValue <" << descriptorInput_->minimumValue()
                                     << ">");
    descriptorRange_.setMinimumValue(descriptorInput_->minimumValue() *
                                     0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorElevation::slotDescriptorIntermediateMaximumValue()
{
    LOG_DEBUG_LOCAL("maximumValue <" << descriptorInput_->maximumValue()
                                     << ">");
    descriptorRange_.setMaximumValue(descriptorInput_->maximumValue() *
                                     0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorElevation::descriptorInputChanged()
{
    LOG_DEBUG_LOCAL("");
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDescriptorRange(descriptorRange_);
    mainWindow_->updateFilter();
}
