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

/** @file QtRadioButton.cpp */

// Include 3D Forest.
#include <QtRadioButton.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtRadioButton"
#include <Log.hpp>

QtRadioButton::QtRadioButton(RadioButton *radioButton, QWidget *parent)
    : QRadioButton(parent),
      radioButton_(radioButton)
{
    // Exclusivity is managed by the common RadioButtonGroup.
    setAutoExclusive(false);

    setText(QString::fromStdString(radioButton_->text()));
    setChecked(radioButton_->isChecked());

    const QPointer<QtRadioButton> guard(this);

    radioButton_->textChanged.connect(
        [guard](const std::string &text)
        {
            if (guard)
            {
                guard->setText(QString::fromStdString(text));
            }
        });

    radioButton_->checkedUpdated.connect(
        [guard](bool checked)
        {
            if (guard)
            {
                const QSignalBlocker blocker(guard.data());
                guard->setChecked(checked);
            }
        });
}

QtRadioButton::~QtRadioButton()
{
}

void QtRadioButton::nextCheckState()
{
    // Mouse/keyboard activation selects the button.
    // Activating an already selected radio button does not uncheck it.
    radioButton_->setChecked(true, true);
}
