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
#include <Application.hpp>
#include <HBoxLayout.hpp>
#include <InfoDialog.hpp>
#include <PushButton.hpp>
#include <TextEdit.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "InfoDialog"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

InfoDialog::InfoDialog(Application *app, int w, int h)
    : Dialog(app),
      defaultWidth_(w),
      defaultHeight_(h),
      textEdit_(nullptr)
{
    LOG_DEBUG(<< "Create.");

    // Text.
    textEdit_ = new TextEdit;
    textEdit_->setFocusPolicy(Ui::NoFocus);
    textEdit_->setReadOnly(true);

    // Buttons.
    closeButton_ = new PushButton(tr("Close"));
    closeButton_->clicked.connect([this]() { slotClose(); });

    HBoxLayout *buttonsLayout = new HBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(closeButton_);

    // Dialog layout.
    VBoxLayout *dialogLayout = new VBoxLayout;
    dialogLayout->addWidget(textEdit_);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(buttonsLayout);

    setLayout(dialogLayout);
}

Size InfoDialog::sizeHint() const
{
    return Size(defaultWidth_, defaultHeight_);
}

Size InfoDialog::minimumSizeHint() const
{
    return Size(defaultWidth_, defaultHeight_);
}

void InfoDialog::setText(const std::string &text)
{
    textEdit_->setText(text);
}

void InfoDialog::slotClose()
{
    hide();
}
