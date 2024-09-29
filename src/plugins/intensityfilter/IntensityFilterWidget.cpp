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

/** @file IntensityFilterWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <IntensityFilterWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "IntensityFilterWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

IntensityFilterWidget::IntensityFilterWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating intensity filter widget.");

    // Input widgets.
    DoubleRangeSliderWidget::create(intensityInput_,
                                    this,
                                    SLOT(slotRangeIntermediateMinimumValue()),
                                    SLOT(slotRangeIntermediateMaximumValue()),
                                    tr("Intensity"),
                                    tr("Min-max intensity range filter"),
                                    tr("%"),
                                    1,
                                    0,
                                    100,
                                    0,
                                    100);

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(intensityInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating intensity filter widget.");
}

void IntensityFilterWidget::slotUpdate(void *sender,
                                       const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_INTENSITY) ||
        target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input intensity filter.");
        setIntensity(mainWindow_->editor().intensityFilter());
    }
}

void IntensityFilterWidget::setIntensity(const Range<double> &range)
{
    LOG_DEBUG(<< "Set intensity filter range <" << range << ">.");

    intensityRange_ = range;
    double min = intensityRange_.minimum() * 100.0;
    double max = intensityRange_.maximum() * 100.0;

    intensityInput_->blockSignals(true);
    intensityInput_->setMinimum(min);
    intensityInput_->setMaximum(max);
    intensityInput_->setMinimumValue(min);
    intensityInput_->setMaximumValue(max);
    intensityInput_->blockSignals(false);
}

void IntensityFilterWidget::filterChanged()
{
    LOG_DEBUG(<< "Intensity filer changed.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setIntensityFilter(intensityRange_);
    mainWindow_->updateFilter();
}

void IntensityFilterWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set intensity filer enabled <" << toString(b) << ">.");

    intensityRange_.setEnabled(b);
    filterChanged();
}

void IntensityFilterWidget::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Minimum value changed.");

    intensityRange_.setMinimumValue(intensityInput_->minimumValue() * 0.01);
    filterChanged();
}

void IntensityFilterWidget::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Maximum value changed.");

    intensityRange_.setMaximumValue(intensityInput_->maximumValue() * 0.01);
    filterChanged();
}
