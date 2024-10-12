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

/** @file TreeTableExportDialog.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>
#include <TreeTableExportDialog.hpp>

// Include Qt.
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "TreeTableExportDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/treetable/", name))

TreeTableExportDialog::TreeTableExportDialog(MainWindow *mainWindow,
                                             const QString &fileName)
    : QDialog(mainWindow),
      mainWindow_(mainWindow)
{
    // File name.
    fileNameLineEdit_ = new QLineEdit;
    fileNameLineEdit_->setText(fileName);

    browseButton_ = new QPushButton(tr("Browse"));
    connect(browseButton_, SIGNAL(clicked()), this, SLOT(slotBrowse()));

    QHBoxLayout *fileNameLayout = new QHBoxLayout;
    fileNameLayout->addWidget(new QLabel(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Buttons.
    acceptButton_ = new QPushButton(tr("Export"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog layout.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window.
    setWindowTitle(tr("Export File"));
    setWindowIcon(THEME_ICON("export_file"));
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void TreeTableExportDialog::slotBrowse()
{
    QFileDialog::Options options;
    options = QFlag(QFileDialog::DontConfirmOverwrite);

    QString selectedFilter;

    QString fileName =
        QFileDialog::getSaveFileName(mainWindow_,
                                     tr("Export File As"),
                                     fileNameLineEdit_->text(),
                                     tr("Comma Separated Values (*.csv)"),
                                     &selectedFilter,
                                     options);

    if (fileName.isEmpty())
    {
        return;
    }

    fileNameLineEdit_->setText(fileName);
}

void TreeTableExportDialog::slotAccept()
{
    QString path = fileNameLineEdit_->text();

    if (path.isEmpty())
    {
        (void)QMessageBox::information(this,
                                       tr("Export File"),
                                       tr("Please choose a file name."));
        return;
    }

    if (File::exists(path.toStdString()))
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this,
                                      tr("Export File"),
                                      tr("Overwrite existing file?"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes)
        {
            return;
        }
    }

    close();
    setResult(QDialog::Accepted);
}

void TreeTableExportDialog::slotReject()
{
    close();
    setResult(QDialog::Rejected);
}

std::string TreeTableExportDialog::fileName() const
{
    return fileNameLineEdit_->text().toStdString();
}
