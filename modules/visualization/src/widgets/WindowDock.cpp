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

/** @file WindowDock.cpp */

#include <QMainWindow>
#include <WindowDock.hpp>
#include <QHBoxLayout>
#include <QLabel>

WindowDock::WindowDock(QMainWindow *parent)
    : QDockWidget(parent),
      mainWindow_(parent)
{
    QWidget *tileBar = new QWidget;
    tileBar->setMinimumSize(20, 20);

    // Widgets
    icon_ = new QLabel;
    title_ = new QLabel;
    title_->setText("Title");

    // Layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(icon_);
    layout->addSpacing(5);
    layout->addWidget(title_);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);
    tileBar->setLayout(layout);

    // Set
    setTitleBarWidget(tileBar);
}

QMainWindow *WindowDock::mainWindow() const
{
    return mainWindow_;
}

void WindowDock::setWindowTitle(const QString &text)
{
    title_->setText(text);
}

void WindowDock::setWindowIcon(const QIcon &icon)
{
    icon_->setPixmap(icon.pixmap(20, 20));
}
