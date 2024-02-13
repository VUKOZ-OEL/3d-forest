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

/** @file InfoDialog.cpp */

// Include 3D Forest.
#include <InfoDialog.hpp>

// Include Qt.
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "InfoDialog"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

InfoDialog::InfoDialog(QWidget *parent, int w, int h)
    : QDialog(parent),
      defaultWidth_(w),
      defaultHeight_(h),
      textEdit_(nullptr)
{
    LOG_DEBUG(<< "Create.");

    // Text.
    textEdit_ = new QTextEdit;
    textEdit_->setFocusPolicy(Qt::NoFocus);
    textEdit_->setReadOnly(true);

    // Buttons.
    closeButton_ = new QPushButton(tr("Close"));
    connect(closeButton_, SIGNAL(clicked()), this, SLOT(slotClose()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(closeButton_);

    // Dialog layout.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(textEdit_);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(buttonsLayout);

    setLayout(dialogLayout);
}

QSize InfoDialog::sizeHint() const
{
    return QSize(defaultWidth_, defaultHeight_);
}

QSize InfoDialog::minimumSizeHint() const
{
    return QSize(defaultWidth_, defaultHeight_);
}

void InfoDialog::setText(const QString &text)
{
    textEdit_->setText(text);
}

void InfoDialog::slotClose()
{
    hide();
}
