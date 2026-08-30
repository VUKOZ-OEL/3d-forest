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

/** @file QtComboBox.cpp */

// Include 3D Forest.
#include <QtComboBox.hpp>

// Include Qt.
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtComboBox"
#include <Log.hpp>

QtComboBox::QtComboBox(ComboBox *comboBox, QWidget *parent)
    : QComboBox(parent),
      comboBox_(comboBox)
{
    for (int i = 0; i < comboBox_->count(); ++i)
    {
        QComboBox::addItem(QString::fromStdString(comboBox_->itemText(i)));
    }

    QComboBox::setCurrentIndex(comboBox_->currentIndex());

    QObject::connect(this,
                     qOverload<int>(&QComboBox::activated),
                     [this](int index) { comboBox_->setValue(index, true); });

    comboBox_->currentIndexChanged.connect(
        [this](int index)
        {
            const QSignalBlocker blocker(this);
            QComboBox::setCurrentIndex(index);
        });

    comboBox_->itemAdded.connect(
        [this](const std::string &text)
        {
            const QSignalBlocker blocker(this);

            QComboBox::addItem(QString::fromStdString(text));

            QComboBox::setCurrentIndex(comboBox_->currentIndex());
        });
}
