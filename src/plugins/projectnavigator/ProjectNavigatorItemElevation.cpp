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

/** @file ProjectNavigatorItemElevation.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorItemElevation.hpp>
#include <RangeSliderWidget.hpp>

#include <QVBoxLayout>

ProjectNavigatorItemElevation::ProjectNavigatorItemElevation(
    MainWindow *mainWindow,
    const QIcon &icon,
    const QString &text)
    : ProjectNavigatorItem(mainWindow, icon, text)
{
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

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(rangeInput_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ProjectNavigatorItemElevation::slotUpdate(void *sender,
                                               const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_ELEVATION))
    {
        elevationRange_ = mainWindow_->editor().elevationRange();

        rangeInput_->blockSignals(true);
        rangeInput_->setMinimum(elevationRange_.minimum());
        rangeInput_->setMaximum(elevationRange_.maximum());
        rangeInput_->setMinimumValue(elevationRange_.minimumValue());
        rangeInput_->setMaximumValue(elevationRange_.maximumValue());
        rangeInput_->blockSignals(false);
    }
}

void ProjectNavigatorItemElevation::slotRangeIntermediateMinimumValue()
{
    elevationRange_.setMinimumValue(rangeInput_->minimumValue());
    elevationInputChanged();
}

void ProjectNavigatorItemElevation::slotRangeIntermediateMaximumValue()
{
    elevationRange_.setMaximumValue(rangeInput_->maximumValue());
    elevationInputChanged();
}

void ProjectNavigatorItemElevation::elevationInputChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setElevationRange(elevationRange_);
    mainWindow_->updateFilter();
}

bool ProjectNavigatorItemElevation::isFilterEnabled() const
{
    return elevationRange_.isEnabled();
}

void ProjectNavigatorItemElevation::setFilterEnabled(bool b)
{
    elevationRange_.setEnabled(b);
    elevationInputChanged();
}
