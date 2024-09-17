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

/** @file ExplorerClippingCylinderWidget.cpp */

// Include 3D Forest.
#include <DoubleSliderWidget.hpp>
#include <ExplorerClippingCylinderWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerClippingCylinderWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ExplorerClippingCylinderWidget::ExplorerClippingCylinderWidget(
    MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Widgets.
    for (size_t i = 0; i < 3; i++)
    {
        DoubleSliderWidget::create(
            pointAInput_[i],
            this,
            nullptr,
            SLOT(slotInputChanged()),
            tr("A"),
            tr("A"),
            tr("m"),
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
            tr("m"),
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
                               tr("m"),
                               1.0,
                               1.0,
                               100.0,
                               1.0,
                               DoubleSliderWidget::LAYOUT_SLIDER_BESIDE_LABEL);

    // Layout.
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

void ExplorerClippingCylinderWidget::setRegion(const Region &region)
{
    LOG_DEBUG(<< "Set region <" << region << ">.");

    double ppm = mainWindow_->editor().settings().units().pointsPerMeter()[0];

    for (size_t i = 0; i < 3; i++)
    {
        double min = region.boundary.min(i) / ppm;
        double max = region.boundary.max(i) / ppm;
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

    pointAInput_[2]->setValue(region.boundary.max(2) / ppm);
    pointBInput_[2]->setValue(region.boundary.min(2) / ppm);

    radiusInput_->blockSignals(true);
    radiusInput_->setMinimum(0.001);
    radiusInput_->setMaximum(region.boundary.length(0) / ppm);
    radiusInput_->setValue(region.boundary.length(0) / ppm);
    radiusInput_->blockSignals(false);
}

void ExplorerClippingCylinderWidget::slotInputChanged()
{
    LOG_DEBUG(<< "Input changed.");

    double ppm = mainWindow_->editor().settings().units().pointsPerMeter()[0];

    double x1 = pointAInput_[0]->value() * ppm;
    double y1 = pointAInput_[1]->value() * ppm;
    double z1 = pointAInput_[2]->value() * ppm;
    double x2 = pointBInput_[0]->value() * ppm;
    double y2 = pointBInput_[1]->value() * ppm;
    double z2 = pointBInput_[2]->value() * ppm;
    double r = radiusInput_->value() * ppm;

    Region region;
    region.cylinder.set(x1, y1, z1, x2, y2, z2, r);
    region.shape = Region::SHAPE_CYLINDER;

    emit signalRegionChanged(region);
}
