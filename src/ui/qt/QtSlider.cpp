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

/** @file QtSlider.cpp */

// Include 3D Forest.
#include <QtSlider.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtSlider"
#include <Log.hpp>

QtSlider::QtSlider(Slider *slider, QWidget *parent)
    : QSlider(Qt::Horizontal, parent),
      slider_(slider)
{
    updateSettings();

    const QPointer<QtSlider> guard(this);

    connect(this,
            &QSlider::valueChanged,
            this,
            [this](int value) { slider_->setValue(value, true); });

    slider_->valueUpdated.connect(
        [guard](int value)
        {
            if (guard)
            {
                const QSignalBlocker blocker(guard.data());
                guard->setValue(value);
            }
        });

    slider_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });
}

QtSlider::~QtSlider()
{
}

void QtSlider::updateSettings()
{
    const QSignalBlocker blocker(this);

    setRange(slider_->minimum(), slider_->maximum());
    setSingleStep(slider_->singleStep());
    setTickInterval(slider_->tickInterval());
    setTickPosition(
        static_cast<QSlider::TickPosition>(slider_->tickPosition()));

    setValue(slider_->value());
}
