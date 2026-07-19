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

/** @file External3dMarteloscopeDialog.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <External3dMarteloscopeDialog.hpp>
#include <FileDialog.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <PushButton.hpp>
#include <ThemeIcon.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopeDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

External3dMarteloscopeDialog::External3dMarteloscopeDialog(Application *app)
    : Dialog(app),
      app_(app)
{
    // File name.
    fileNameLineEdit_ = new LineEdit;
    fileNameLineEdit_->setText(path_);

    browseButton_ = new PushButton(tr("Browse"));
    browseButton_->clicked.connect([this]() { slotBrowse(); });

    HBoxLayout *fileNameLayout = new HBoxLayout;
    fileNameLayout->addWidget(new Label(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Dialog buttons.
    acceptButton_ = new PushButton(tr("Run"));
    acceptButton_->setIcon(THEME_ICON("run"));
    acceptButton_->clicked.connect([this]() { slotAccept(); });

    rejectButton_ = new PushButton(tr("Cancel"));
    rejectButton_->clicked.connect([this]() { slotReject(); });

    HBoxLayout *dialogButtons = new HBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog.
    VBoxLayout *dialogLayout = new VBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    setWindowTitle(tr("Run iLand"));
    setWindowIcon(THEME_ICON("run"));
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void External3dMarteloscopeDialog::slotBrowse()
{
    std::string selectedFilter;

    std::string fileName =
        FileDialog::getSaveFileName(app_,
                                    tr("Select File"),
                                    fileNameLineEdit_->text(),
                                    tr("iLand project XML (*.xml)"),
                                    &selectedFilter,
                                    FileDialog::DontConfirmOverwrite);

    if (fileName.empty())
    {
        return;
    }

    fileNameLineEdit_->setText(fileName);
}

void External3dMarteloscopeDialog::slotAccept()
{
    path_ = fileNameLineEdit_->text();

    close();
    setResult(Dialog::Accepted);
}

void External3dMarteloscopeDialog::slotReject()
{
    close();
    setResult(Dialog::Rejected);
}
