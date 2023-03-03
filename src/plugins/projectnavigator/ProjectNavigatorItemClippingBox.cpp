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

/** @file ProjectNavigatorItemClippingBox.cpp */

#include <DoubleRangeSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorItemClippingBox.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ProjectNavigatorItemClippingBox"
#include <Log.hpp>

ProjectNavigatorItemClippingBox::ProjectNavigatorItemClippingBox(
    MainWindow *mainWindow)
    : QWidget(mainWindow)
{
    LOG_DEBUG(<< "Called.");

    // Widgets
    DoubleRangeSliderWidget::create(
        rangeInput_[0],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        tr("X range"),
        tr("Min-max clipping range filter along X axis"),
        tr("pt"),
        1,
        0,
        100,
        0,
        100);

    DoubleRangeSliderWidget::create(
        rangeInput_[1],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        tr("Y range"),
        tr("Min-max clipping range filter along Y axis"),
        tr("pt"),
        1,
        0,
        100,
        0,
        100);

    DoubleRangeSliderWidget::create(
        rangeInput_[2],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        tr("Z range"),
        tr("Min-max clipping range filter along Z axis"),
        tr("pt"),
        1,
        0,
        100,
        0,
        100);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(rangeInput_[0]);
    mainLayout->addWidget(rangeInput_[1]);
    mainLayout->addWidget(rangeInput_[2]);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void ProjectNavigatorItemClippingBox::setRegion(const Region &region)
{
    LOG_DEBUG(<< "Set region <" << region << ">.");

    for (size_t i = 0; i < 3; i++)
    {
        clipRange_[i].setMinimum(region.boundary.min(i));
        clipRange_[i].setMaximum(region.boundary.max(i));
        clipRange_[i].setMinimumValue(region.box.min(i));
        clipRange_[i].setMaximumValue(region.box.max(i));

        rangeInput_[i]->blockSignals(true);
        rangeInput_[i]->setMinimum(clipRange_[i].minimum());
        rangeInput_[i]->setMaximum(clipRange_[i].maximum());
        rangeInput_[i]->setMinimumValue(clipRange_[i].minimumValue());
        rangeInput_[i]->setMaximumValue(clipRange_[i].maximumValue());
        rangeInput_[i]->blockSignals(false);
    }
}

void ProjectNavigatorItemClippingBox::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Called.");
    QObject *obj = sender();
    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeInput_[i])
        {
            double v = rangeInput_[i]->minimumValue();
            LOG_DEBUG(<< "Input minimumValue <" << v << ">.");
            clipRange_[i].setMinimumValue(v);
        }
    }

    updateRegion();
}

void ProjectNavigatorItemClippingBox::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Called.");
    QObject *obj = sender();
    for (int i = 0; i < 3; i++)
    {
        if (obj == rangeInput_[i])
        {
            double v = rangeInput_[i]->maximumValue();
            LOG_DEBUG(<< "Input maximumValue <" << v << ">.");
            clipRange_[i].setMaximumValue(v);
        }
    }

    updateRegion();
}

void ProjectNavigatorItemClippingBox::updateRegion()
{
    LOG_DEBUG(<< "Called.");

    double x1 = clipRange_[0].minimumValue();
    double y1 = clipRange_[1].minimumValue();
    double z1 = clipRange_[2].minimumValue();
    double x2 = clipRange_[0].maximumValue();
    double y2 = clipRange_[1].maximumValue();
    double z2 = clipRange_[2].maximumValue();

    Region region;
    region.box.set(x1, y1, z1, x2, y2, z2);
    region.enabled = Region::TYPE_BOX;

    emit signalRegionChanged(region);
}
