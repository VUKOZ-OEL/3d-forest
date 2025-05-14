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

/** @file FilterAreaBoxWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <FilterAreaBoxWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterAreaBoxWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterAreaBoxWidget::FilterAreaBoxWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating clip filter widget.");

    // Widgets.
    DoubleRangeSliderWidget::create(
        rangeInput_[0],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        SLOT(slotFinalValue()),
        tr("X range"),
        tr("Min-max clipping range filter along X axis"),
        tr("m"),
        0.01,
        0,
        100,
        0,
        100);

    DoubleRangeSliderWidget::create(
        rangeInput_[1],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        SLOT(slotFinalValue()),
        tr("Y range"),
        tr("Min-max clipping range filter along Y axis"),
        tr("m"),
        0.01,
        0,
        100,
        0,
        100);

    DoubleRangeSliderWidget::create(
        rangeInput_[2],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        SLOT(slotFinalValue()),
        tr("Z range"),
        tr("Min-max clipping range filter along Z axis"),
        tr("m"),
        0.01,
        0,
        100,
        0,
        100);

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(rangeInput_[0]);
    mainLayout->addWidget(rangeInput_[1]);
    mainLayout->addWidget(rangeInput_[2]);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating clip filter widget.");
}

void FilterAreaBoxWidget::slotUpdate(void *sender,
                                     const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_CLIP_FILTER) ||
        target.contains(Editor::TYPE_SETTINGS) ||
        target.contains(Editor::TYPE_DATA_SET))
    {
        LOG_DEBUG_UPDATE(<< "Input clip box filter.");
        setRegion(mainWindow_->editor().clipFilter());
    }
}

void FilterAreaBoxWidget::setRegion(const Region &region)
{
    LOG_DEBUG(<< "Set clip box filter region <" << region << ">.");

    region_ = region;

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    for (size_t i = 0; i < 3; i++)
    {
        clipRange_[i].setMinimum(region.boundary.min(i));
        clipRange_[i].setMaximum(region.boundary.max(i));
        clipRange_[i].setMinimumValue(region.box.min(i));
        clipRange_[i].setMaximumValue(region.box.max(i));

        rangeInput_[i]->blockSignals(true);
        rangeInput_[i]->setMinimum(clipRange_[i].minimum() / ppm);
        rangeInput_[i]->setMaximum(clipRange_[i].maximum() / ppm);
        rangeInput_[i]->setMinimumValue(clipRange_[i].minimumValue() / ppm);
        rangeInput_[i]->setMaximumValue(clipRange_[i].maximumValue() / ppm);
        rangeInput_[i]->blockSignals(false);
    }
}

void FilterAreaBoxWidget::filterChanged(bool final)
{
    LOG_DEBUG(<< "Clip box filer changed.");

    double x1 = clipRange_[0].minimumValue();
    double y1 = clipRange_[1].minimumValue();
    double z1 = clipRange_[2].minimumValue();
    double x2 = clipRange_[0].maximumValue();
    double y2 = clipRange_[1].maximumValue();
    double z2 = clipRange_[2].maximumValue();

    region_.box.set(x1, y1, z1, x2, y2, z2);
    region_.shape = Region::Shape::BOX;

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClipFilter(region_);
    mainWindow_->updateFilter(this, final);
}

void FilterAreaBoxWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set clip box filer enabled <" << toString(b) << ">.");

    if (b)
    {
        region_.shape = Region::Shape::BOX;
    }
    else
    {
        region_.shape = Region::Shape::NONE;
    }

    mainWindow_->suspendThreads();
    mainWindow_->editor().setClipFilter(region_);
    mainWindow_->updateFilter(this, true);
}

void FilterAreaBoxWidget::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Minimum value changed.");

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    QObject *obj = sender();
    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeInput_[i])
        {
            double v = rangeInput_[i]->minimumValue();
            LOG_DEBUG(<< "Input minimumValue <" << v << ">.");
            clipRange_[i].setMinimumValue(v * ppm);
        }
    }

    filterChanged(false);
}

void FilterAreaBoxWidget::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Maximum value changed.");

    double ppm =
        mainWindow_->editor().settings().unitsSettings().pointsPerMeter()[0];

    QObject *obj = sender();
    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeInput_[i])
        {
            double v = rangeInput_[i]->maximumValue();
            LOG_DEBUG(<< "Input maximumValue <" << v << ">.");
            clipRange_[i].setMaximumValue(v * ppm);
        }
    }

    filterChanged(false);
}

void FilterAreaBoxWidget::slotFinalValue()
{
    filterChanged(true);
}

void FilterAreaBoxWidget::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    QWidget::showEvent(event);
}

void FilterAreaBoxWidget::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide event.");
    QWidget::hideEvent(event);
}
