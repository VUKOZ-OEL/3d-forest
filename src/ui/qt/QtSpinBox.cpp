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

/** @file QtSpinBox.cpp */

// Include 3D Forest.
#include <QtSpinBox.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtSpinBox"
#include <Log.hpp>

QtSpinBox::QtSpinBox(SpinBox *spinBox, QWidget *parent)
    : QSpinBox(parent),
      spinBox_(spinBox)
{
    updateSettings();

    const QPointer<QtSpinBox> guard(this);

    // Qt user changes -> common.
    QObject::connect(this,
                     &QSpinBox::valueChanged,
                     this,
                     [this](int value) { spinBox_->setValue(value, true); });

    QObject::connect(this,
                     &QSpinBox::editingFinished,
                     this,
                     [this]
                     {
                         if (!spinBox_->signalsBlocked())
                         {
                             spinBox_->editingFinished();
                         }
                     });

    // Common -> Qt.
    spinBox_->valueUpdated.connect(
        [guard](int value)
        {
            if (!guard || guard->value() == value)
            {
                return;
            }

            const QSignalBlocker blocker(guard.data());
            guard->setValue(value);
        });

    spinBox_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });
}

QtSpinBox::~QtSpinBox()
{
}

void QtSpinBox::updateSettings()
{
    const QSignalBlocker blocker(this);

    setRange(spinBox_->minimum(), spinBox_->maximum());
    setSingleStep(spinBox_->singleStep());
    setValue(spinBox_->value());
}
