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

/** @file QtGroupBox.cpp */

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtGroupBox.hpp>

// Include Qt.
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtGroupBox"
#include <Log.hpp>

QtGroupBox::QtGroupBox(GroupBox *groupBox, QtApplication *app, QWidget *parent)
    : QGroupBox(parent),
      groupBox_(groupBox)
{
    setTitle(QString::fromStdString(groupBox_->title()));

    groupBox_->titleChanged.connect(
        [this](const std::string &title)
        { setTitle(QString::fromStdString(title)); });

    if (groupBox_->layout())
    {
        QLayout *layout = app->createLayout(groupBox_->layout(), this);

        if (layout)
        {
            QGroupBox::setLayout(layout);
        }
    }
}

QtGroupBox::~QtGroupBox()
{
}
