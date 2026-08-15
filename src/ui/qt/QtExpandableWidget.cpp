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

/** @file QtExpandableWidget.cpp */

// Include 3D Forest.
#include <QtExpandableWidget.hpp>

// Include Qt.
#include <QToolButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "QtExpandableWidget"
#include <Log.hpp>

QtExpandableWidget::QtExpandableWidget(const std::string &title,
                                       QWidget *content,
                                       QWidget *parent)
    : QWidget(parent),
      header_(new QToolButton(this)),
      content_(content)
{
    header_->setText(QString::fromStdString(title));
    header_->setCheckable(true);
    header_->setChecked(true);
    header_->setArrowType(Qt::DownArrow);
    header_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    header_->setSizePolicy(QSizePolicy::Expanding,
                           QSizePolicy::Fixed); // QSizePolicy::Preferred);

    header_->setMinimumHeight(28);

    header_->setStyleSheet("QToolButton {"
                           "    text-align: left;"
                           "    padding: 6px;"
                           "    border: none;"
                           "    font-weight: bold;"
                           "}");

    content_->setParent(this);
    content_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(header_);
    layout->addWidget(content_);

    QObject::connect(header_,
                     &QToolButton::toggled,
                     this,
                     [this](bool expanded)
                     {
                         content_->setVisible(expanded);

                         header_->setArrowType(expanded ? Qt::DownArrow
                                                        : Qt::RightArrow);
                     });

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}
