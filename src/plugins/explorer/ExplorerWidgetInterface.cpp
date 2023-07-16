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

/** @file ExplorerWidgetInterface.cpp */

#include <ExplorerWidgetInterface.hpp>
#include <MainWindow.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#define LOG_MODULE_NAME "ExplorerWidgetInterface"
#include <Log.hpp>

ExplorerWidgetInterface::ExplorerWidgetInterface(MainWindow *mainWindow,
                                                 const QIcon &icon,
                                                 const QString &text)
    : QWidget(),
      mainWindow_(mainWindow),
      icon_(icon),
      text_(text),
      titleIcon_(nullptr),
      titleText_(nullptr),
      mainLayout_(nullptr),
      filterEnabled_(true)
{
#if 0
    // Title
    titleIcon_ = new QLabel;
    titleIcon_->setPixmap(icon_.pixmap(icon_.actualSize(QSize(16, 16))));

    titleText_ = new QLabel;
    titleText_->setText(text_);

    QHBoxLayout *titleBar = new QHBoxLayout;
    titleBar->addWidget(titleIcon_);
    titleBar->addWidget(titleText_);
    titleBar->addStretch();

    // Layout
    mainLayout_ = new QVBoxLayout;
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->addLayout(titleBar);
#else
    // Layout
    mainLayout_ = new QVBoxLayout;
    mainLayout_->setContentsMargins(0, 0, 0, 0);
#endif
}
