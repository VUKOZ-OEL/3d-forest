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
#include <External3dMarteloscopeDialog.hpp>
#include <Application.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QFileDialog>
#include <HBoxLayout>
#include <Label>
#include <QLineEdit>
#include <PushButton>
#include <VBoxLayout>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopeDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

External3dMarteloscopeDialog::External3dMarteloscopeDialog(
    Application *app)
    : Dialog(app),
      app_(app)
{
    // File name.
    fileNameLineEdit_ = new QLineEdit;
    fileNameLineEdit_->setText(QString::fromStdString(path_));

    browseButton_ = new PushButton(tr("Browse"));
    connect(browseButton_, SIGNAL(clicked()), this, SLOT(slotBrowse()));

    HBoxLayout *fileNameLayout = new HBoxLayout;
    fileNameLayout->addWidget(new Label(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Dialog buttons.
    acceptButton_ = new PushButton(tr("Run"));
    acceptButton_->setIcon(THEME_ICON("run"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new PushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

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
    QFileDialog::Options options;
    options = QFlag(QFileDialog::DontConfirmOverwrite);

    QString selectedFilter;

    QString fileName =
        QFileDialog::getSaveFileName(app_,
                                     tr("Select File"),
                                     fileNameLineEdit_->text(),
                                     tr("iLand project XML (*.xml)"),
                                     &selectedFilter,
                                     options);

    if (fileName.isEmpty())
    {
        return;
    }

    fileNameLineEdit_->setText(fileName);
}

void External3dMarteloscopeDialog::slotAccept()
{
    path_ = fileNameLineEdit_->text().toStdString();

    close();
    setResult(Dialog::Accepted);
}

void External3dMarteloscopeDialog::slotReject()
{
    close();
    setResult(Dialog::Rejected);
}
