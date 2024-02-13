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

/** @file ShowDetailsWidget.cpp */

// Include 3D Forest.
#include <ShowDetailsWidget.hpp>

// Include Qt.
#include <QFrame>
#include <QTextEdit>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ShowDetailsWidget"
#include <Log.hpp>

ShowDetailsWidget::ShowDetailsWidget(const QString &text)
    : QWidget(),
      textEdit_(nullptr)
{
    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    textEdit_ = new QTextEdit;
    textEdit_->setFixedHeight(100);
    textEdit_->setFocusPolicy(Qt::NoFocus);
    textEdit_->setReadOnly(true);
    textEdit_->setText(text);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(QMargins());
    layout->addWidget(line);
    layout->addWidget(textEdit_);

    setLayout(layout);
}

void ShowDetailsWidget::create(ShowDetailsWidget *&outputWidget,
                               const QString &text)
{
    outputWidget = new ShowDetailsWidget(text);
}
