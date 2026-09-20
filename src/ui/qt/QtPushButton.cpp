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

/** @file QtPushButton.cpp */

// Include 3D Forest.
#include <QtPushButton.hpp>

// Include Qt.
#include <QPointer>
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtPushButton"
#include <Log.hpp>

QtPushButton::QtPushButton(PushButton *pushButton, QWidget *parent)
    : QPushButton(QString::fromStdString(pushButton->text()), parent),
      pushButton_(pushButton)
{
    setText(QString::fromStdString(pushButton_->text()));
    // setIcon(iconConverter(pushButton_->icon()));

    const QPointer<QtPushButton> guard(this);

    pushButton_->textChanged.connect(
        [guard](const std::string &text)
        {
            if (guard)
            {
                guard->setText(QString::fromStdString(text));
            }
        });

    pushButton_->iconChanged.connect(
        [guard](const ThemeIcon &icon)
        {
            if (guard)
            {
                // guard->setIcon(iconConverter(icon));
            }
        });

    QObject::connect(this,
                     &QPushButton::clicked,
                     this,
                     [this](bool) { pushButton_->click(); });
}

QtPushButton::~QtPushButton()
{
}
