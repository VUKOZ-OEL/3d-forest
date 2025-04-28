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

/** @file InputComboBoxDialog.cpp */

// Include 3D Forest.
#include <InputComboBoxDialog.hpp>

// Include Qt.
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "InputComboBoxDialog"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

InputComboBoxDialog::InputComboBoxDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Select a value");

    comboBox = new QComboBox(this);

    okButton = new QPushButton("Ok", this);
    cancelButton = new QPushButton("Cancel", this);

    connect(okButton,
            &QPushButton::clicked,
            this,
            &InputComboBoxDialog::accept);
    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &InputComboBoxDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(comboBox);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void InputComboBoxDialog::setOkButtonText(const QString &text)
{
    okButton->setText(text);
}

void InputComboBoxDialog::addItem(const QString &text)
{
    comboBox->addItem(text);
}

int InputComboBoxDialog::currentIndex() const
{
    return comboBox->currentIndex();
}

QString InputComboBoxDialog::currentText() const
{
    return comboBox->currentText();
}
