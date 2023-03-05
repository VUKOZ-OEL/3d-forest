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

/** @file ProjectNavigatorItemClippingCylinder.cpp */

#include <DoubleSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorItemClippingCylinder.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ProjectNavigatorItemClippingCylinder"
#include <Log.hpp>

ProjectNavigatorItemClippingCylinder::ProjectNavigatorItemClippingCylinder(
    MainWindow *mainWindow)
    : QWidget(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Widgets
    for (size_t i = 0; i < 3; i++)
    {
        DoubleSliderWidget::create(
            pointAInput_[i],
            this,
            nullptr,
            SLOT(slotInputChanged()),
            tr("A"),
            tr("A"),
            tr("pt"),
            1.0,
            1.0,
            100.0,
            1.0,
            DoubleSliderWidget::LAYOUT_SLIDER_BESIDE_LABEL);

        DoubleSliderWidget::create(
            pointBInput_[i],
            this,
            nullptr,
            SLOT(slotInputChanged()),
            tr("B"),
            tr("B"),
            tr("pt"),
            1.0,
            1.0,
            100.0,
            1.0,
            DoubleSliderWidget::LAYOUT_SLIDER_BESIDE_LABEL);
    }

    DoubleSliderWidget::create(radiusInput_,
                               this,
                               nullptr,
                               SLOT(slotInputChanged()),
                               tr("Radius"),
                               tr("Radius"),
                               tr("pt"),
                               1.0,
                               1.0,
                               100.0,
                               1.0,
                               DoubleSliderWidget::LAYOUT_SLIDER_BESIDE_LABEL);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    for (size_t i = 0; i < 3; i++)
    {
        mainLayout->addWidget(pointAInput_[i]);
    }
    for (size_t i = 0; i < 3; i++)
    {
        mainLayout->addWidget(pointBInput_[i]);
    }
    mainLayout->addWidget(radiusInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void ProjectNavigatorItemClippingCylinder::setRegion(const Region &region)
{
    LOG_DEBUG(<< "Set region <" << region << ">.");

    for (size_t i = 0; i < 3; i++)
    {
        double min = region.boundary.min(i);
        double max = region.boundary.max(i);
        double value = min + (0.5 * (max - min));

        pointAInput_[i]->blockSignals(true);
        pointAInput_[i]->setMinimum(min);
        pointAInput_[i]->setMaximum(max);
        pointAInput_[i]->setValue(value);
        pointAInput_[i]->blockSignals(false);

        pointBInput_[i]->blockSignals(true);
        pointBInput_[i]->setMinimum(min);
        pointBInput_[i]->setMaximum(max);
        pointBInput_[i]->setValue(value);
        pointBInput_[i]->blockSignals(false);
    }

    pointAInput_[2]->setValue(pointAInput_[2]->minimum());
    pointBInput_[2]->setValue(pointBInput_[2]->maximum());

    radiusInput_->blockSignals(true);
    radiusInput_->setMinimum(1);
    radiusInput_->setMaximum(region.boundary.length(0));
    radiusInput_->setValue(radiusInput_->maximum());
    radiusInput_->blockSignals(false);
}

void ProjectNavigatorItemClippingCylinder::slotInputChanged()
{
    LOG_DEBUG(<< "Called.");

    double x1 = pointAInput_[0]->value();
    double y1 = pointAInput_[1]->value();
    double z1 = pointAInput_[2]->value();
    double x2 = pointBInput_[0]->value();
    double y2 = pointBInput_[1]->value();
    double z2 = pointBInput_[2]->value();
    double r = radiusInput_->value();

    Region region;
    region.cylinder.set(x1, y1, z1, x2, y2, z2, r);
    region.enabled = Region::TYPE_CYLINDER;

    emit signalRegionChanged(region);
}
