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

/** @file ExampleWidget.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ExampleWidget.hpp>
#include <Label.hpp>
#include <Slider.hpp>
#include <Ui.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "ExampleWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/ExampleResources/", name))

ExampleWidget::ExampleWidget(Application *app) : app_(app)
{
    LOG_DEBUG(<< "Start creating example widget.");

    // Point size.
    pointSizeSlider_ = new Slider;
    pointSizeSlider_->setMinimum(1);
    pointSizeSlider_->setMaximum(5);
    pointSizeSlider_->setSingleStep(1);
    pointSizeSlider_->setTickInterval(1);
    pointSizeSlider_->setTickPosition(Slider::TicksAbove);
    pointSizeSlider_->setOrientation(Ui::Horizontal);
    pointSizeSlider_->valueChanged.connect([this](int value)
                                           { slotSetPointSize(value); });

    // Layout.
    VBoxLayout *layout = new VBoxLayout;
    layout->addWidget(new Label(tr("Point size")));
    layout->addWidget(pointSizeSlider_);

    setLayout(layout);

    LOG_DEBUG(<< "Finished creating example widget.");
}

void ExampleWidget::slotSetPointSize(int v)
{
    LOG_DEBUG(<< "Set point size <" << v << ">.");
}
