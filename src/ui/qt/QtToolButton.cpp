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

/** @file QtToolButton.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <QtIcon.hpp>
#include <QtToolButton.hpp>

// Include Qt.
#include <QIcon>
#include <QPointer>
#include <QSignalBlocker>
#include <QString>

// Include local.
#define LOG_MODULE_NAME "QtToolButton"
#include <Log.hpp>

namespace
{
Qt::ToolButtonStyle toQtStyle(ToolButton::ToolButtonStyle style)
{
    switch (style)
    {
        case ToolButton::IconOnly:
            return Qt::ToolButtonIconOnly;

        case ToolButton::TextOnly:
            return Qt::ToolButtonTextOnly;

        case ToolButton::TextBesideIcon:
            return Qt::ToolButtonTextBesideIcon;

        case ToolButton::TextUnderIcon:
            return Qt::ToolButtonTextUnderIcon;
    }

    return Qt::ToolButtonIconOnly;
}
} // namespace

QtToolButton::QtToolButton(ToolButton *button, QWidget *parent)
    : QToolButton(parent),
      button_(button)
{
    if (!button_)
    {
        throw std::invalid_argument("QtToolButton: button is null.");
    }

    updateSettings();

    QObject::connect(this,
                     &QToolButton::clicked,
                     this,
                     [this](bool)
                     {
                         if (!button_->signalsBlocked())
                         {
                             button_->clicked();
                         }
                     });

    const QPointer<QtToolButton> guard(this);

    button_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    // Remove the Qt representation before common data disappears.
    button_->destroying.connect(
        [guard]
        {
            if (guard)
            {
                delete guard.data();
            }
        });
}

QtToolButton::~QtToolButton()
{
}

void QtToolButton::updateSettings()
{
    const QSignalBlocker blocker(this);

    QToolButton::setText(QString::fromStdString(button_->text()));
    QToolButton::setToolTip(QString::fromStdString(button_->toolTip()));
    QToolButton::setIcon(toQIcon(button_->icon(), false));
    QToolButton::setToolButtonStyle(toQtStyle(button_->toolButtonStyle()));
    QToolButton::setAutoRaise(button_->autoRaise());

    setAccessibleName(QString::fromStdString(button_->text()));
}
