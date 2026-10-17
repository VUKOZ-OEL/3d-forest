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

/** @file QtDoubleSpinBox.cpp */

// Include 3D Forest.
#include <QtDoubleSpinBox.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtDoubleSpinBox"
#include <Log.hpp>

QtDoubleSpinBox::QtDoubleSpinBox(DoubleSpinBox *doubleSpinBox, QWidget *parent)
    : QDoubleSpinBox(parent),
      doubleSpinBox_(doubleSpinBox)
{
    updateSettings();

    const QPointer<QtDoubleSpinBox> guard(this);

    // Qt user changes -> common.
    QObject::connect(this,
                     &QDoubleSpinBox::valueChanged,
                     this,
                     [this](double value)
                     { doubleSpinBox_->setValue(value, true); });

    QObject::connect(this,
                     &QDoubleSpinBox::editingFinished,
                     this,
                     [this]
                     {
                         if (!doubleSpinBox_->signalsBlocked())
                         {
                             doubleSpinBox_->editingFinished();
                         }
                     });

    // Common -> Qt.
    doubleSpinBox_->valueUpdated.connect(
        [guard](double value)
        {
            if (!guard || guard->value() == value)
            {
                return;
            }

            const QSignalBlocker blocker(guard.data());
            guard->setValue(value);
        });

    doubleSpinBox_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });
}

QtDoubleSpinBox::~QtDoubleSpinBox()
{
}

void QtDoubleSpinBox::updateSettings()
{
    const QSignalBlocker blocker(this);

    setRange(doubleSpinBox_->minimum(), doubleSpinBox_->maximum());
    setSingleStep(doubleSpinBox_->singleStep());
    setValue(doubleSpinBox_->value());
}
