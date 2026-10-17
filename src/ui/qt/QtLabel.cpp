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

/** @file QtLabel.cpp */

// Include 3D Forest.
#include <QtApplication.hpp>
#include <QtIcon.hpp>
#include <QtLabel.hpp>

// Include Qt.
#include <QSignalBlocker>

// Include local.
#define LOG_MODULE_NAME "QtLabel"
#include <Log.hpp>

QtLabel::QtLabel(Label *label, QtApplication *app, QWidget *parent)
    : QLabel(QString::fromStdString(label->text()), parent),
      label_(label),
      app_(app)
{
    updateSettings();

    const QPointer<QtLabel> guard(this);

    label_->settingsChanged.connect(
        [guard]
        {
            if (guard)
            {
                guard->updateSettings();
            }
        });

    // Theme colors.
    app_->bindTheme(this, [this](bool) { updateIcon(); });
}

QtLabel::~QtLabel()
{
}

void QtLabel::updateSettings()
{
    const QSignalBlocker blocker(this);
    updateIcon();
}

void QtLabel::updateIcon()
{
    if (!label_->icon().isNull())
    {
        const QIcon icon = toQIcon(label_->icon(), app_->isDarkMode());

        const int size = Application::ICON_SIZE_TEXT;

        const QPixmap pixmap =
            icon.pixmap(QSize(size, size), devicePixelRatioF());

        if (!pixmap.isNull())
        {
            QLabel::setPixmap(pixmap);
            return;
        }
    }

    QLabel::setText(QString::fromStdString(label_->text()));
    QLabel::setToolTip(QString::fromStdString(label_->toolTip()));
}
