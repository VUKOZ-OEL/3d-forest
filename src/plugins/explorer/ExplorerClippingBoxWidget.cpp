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

/** @file ExplorerClippingBoxWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <ExplorerClippingBoxWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerClippingBoxWidget"
#include <Log.hpp>

ExplorerClippingBoxWidget::ExplorerClippingBoxWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    DoubleRangeSliderWidget::create(
        rangeInput_[0],
        this,
        SLOT(slotRangeIntermediateMinimumValue()),
        SLOT(slotRangeIntermediateMaximumValue()),
        tr("X range"),
        tr("Min-max clipping range filter along X axis"),
        tr("m"),
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
        tr("m"),
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
        tr("m"),
        1,
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
}

void ExplorerClippingBoxWidget::setRegion(const Region &region)
{
    LOG_DEBUG(<< "Set region <" << region << ">.");

    double ppm = mainWindow_->editor().settings().units.pointsPerMeter()[0];

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

void ExplorerClippingBoxWidget::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Minimum value changed.");
    double ppm = mainWindow_->editor().settings().units.pointsPerMeter()[0];
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

    updateRegion();
}

void ExplorerClippingBoxWidget::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Maximum value changed.");
    double ppm = mainWindow_->editor().settings().units.pointsPerMeter()[0];
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

    updateRegion();
}

void ExplorerClippingBoxWidget::updateRegion()
{
    LOG_DEBUG(<< "Update.");

    double x1 = clipRange_[0].minimumValue();
    double y1 = clipRange_[1].minimumValue();
    double z1 = clipRange_[2].minimumValue();
    double x2 = clipRange_[0].maximumValue();
    double y2 = clipRange_[1].maximumValue();
    double z2 = clipRange_[2].maximumValue();

    Region region;
    region.box.set(x1, y1, z1, x2, y2, z2);
    region.shape = Region::SHAPE_BOX;

    emit signalRegionChanged(region);
}
