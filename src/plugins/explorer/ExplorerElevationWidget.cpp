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

/** @file ExplorerElevationWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <ExplorerElevationWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerElevationWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ExplorerElevationWidget::ExplorerElevationWidget(MainWindow *mainWindow,
                                                 const QIcon &icon,
                                                 const QString &text)
    : ExplorerWidgetInterface(mainWindow, icon, text)
{
    // Input widgets.
    DoubleRangeSliderWidget::create(rangeInput_,
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

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(rangeInput_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ExplorerElevationWidget::slotUpdate(void *sender,
                                         const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_ELEVATION))
    {
        setElevation(mainWindow_->editor().elevationFilter());
    }
}

void ExplorerElevationWidget::setElevation(const Range<double> &elevationRange)
{
    LOG_DEBUG(<< "Input elevation <" << elevationRange << ">.");

    rangeInput_->blockSignals(true);

    elevationRange_ = elevationRange;

    rangeInput_->setMinimum(elevationRange_.minimum());
    rangeInput_->setMaximum(elevationRange_.maximum());
    rangeInput_->setMinimumValue(elevationRange_.minimumValue());
    rangeInput_->setMaximumValue(elevationRange_.maximumValue());
    rangeInput_->setDisabled(elevationRange_.empty());

    rangeInput_->blockSignals(false);
}

void ExplorerElevationWidget::filterChanged()
{
    LOG_DEBUG(<< "Output elevation <" << elevationRange_ << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setElevationFilter(elevationRange_);
    mainWindow_->updateFilter();
}

void ExplorerElevationWidget::slotRangeIntermediateMinimumValue()
{
    elevationRange_.setMinimumValue(rangeInput_->minimumValue());
    filterChanged();
}

void ExplorerElevationWidget::slotRangeIntermediateMaximumValue()
{
    elevationRange_.setMaximumValue(rangeInput_->maximumValue());
    filterChanged();
}

bool ExplorerElevationWidget::isFilterEnabled() const
{
    return elevationRange_.isEnabled();
}

void ExplorerElevationWidget::setFilterEnabled(bool b)
{
    elevationRange_.setEnabled(b);
    filterChanged();
}
