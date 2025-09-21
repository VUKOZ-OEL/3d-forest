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
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopeDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

External3dMarteloscopeDialog::External3dMarteloscopeDialog(
    MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow)
{
    // File name.
    fileNameLineEdit_ = new QLineEdit;
    fileNameLineEdit_->setText(QString::fromStdString(path_));

    browseButton_ = new QPushButton(tr("Browse"));
    connect(browseButton_, SIGNAL(clicked()), this, SLOT(slotBrowse()));

    QHBoxLayout *fileNameLayout = new QHBoxLayout;
    fileNameLayout->addWidget(new QLabel(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Dialog buttons.
    acceptButton_ = new QPushButton(tr("Run"));
    acceptButton_->setIcon(THEME_ICON("run").icon());
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    setWindowTitle(tr("Run iLand"));
    setWindowIcon(THEME_ICON("run").icon());
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void External3dMarteloscopeDialog::slotBrowse()
{
    QFileDialog::Options options;
    options = QFlag(QFileDialog::DontConfirmOverwrite);

    QString selectedFilter;

    QString fileName =
        QFileDialog::getSaveFileName(mainWindow_,
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
    setResult(QDialog::Accepted);
}

void External3dMarteloscopeDialog::slotReject()
{
    close();
    setResult(QDialog::Rejected);
}
