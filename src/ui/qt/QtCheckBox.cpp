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

/** @file QtCheckBox.cpp */

// Include 3D Forest.
#include <QtCheckBox.hpp>

// Include Qt.
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtCheckBox"
#include <Log.hpp>

QtCheckBox::QtCheckBox(CheckBox *checkBox, QWidget *parent)
    : QCheckBox(QString::fromStdString(checkBox->text()), parent),
      checkBox_(checkBox)
{
    setChecked(checkBox_->isChecked());

    QObject::connect(this,
                     &QCheckBox::toggled,
                     [this](bool checked) { checkBox_->setChecked(checked); });

    checkBox_->stateChanged.connect(
        [this](int checked)
        {
            const QSignalBlocker blocker(this);
            setChecked(checked);
        });

    checkBox_->textChanged.connect([this](const std::string &text)
                                   { setText(QString::fromStdString(text)); });
}

QtCheckBox::~QtCheckBox()
{
}
