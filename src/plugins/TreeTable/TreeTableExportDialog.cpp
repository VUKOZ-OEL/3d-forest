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
#include <FileFormatCsv.hpp>
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <TreeTableExportDialog.hpp>

// Include Qt.
#include <CheckBox.hpp>
#include <FileDialog.hpp>
#include <GridLayout.hpp>
#include <GroupBox.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <MessageBox.hpp>
#include <PushButton.hpp>
#include <VBoxLayout.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeTableExportDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/TreeTableResources/", name))

TreeTableExportDialog::TreeTableExportDialog(Application *app,
                                             const std::string &fileName)
    : Dialog(app),
      app_(app)
{
    // File name.
    fileNameLineEdit_ = new LineEdit;
    fileNameLineEdit_->setText(fileName);

    browseButton_ = new PushButton(tr("Browse"));
    connect(browseButton_, SIGNAL(clicked()), this, SLOT(slotBrowse()));

    HBoxLayout *fileNameLayout = new HBoxLayout;
    fileNameLayout->addWidget(new Label(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Buttons.
    acceptButton_ = new PushButton(tr("Export"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new PushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    HBoxLayout *dialogButtons = new HBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog layout.
    VBoxLayout *dialogLayout = new VBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window.
    setWindowTitle(tr("Export File"));
    setWindowIcon(THEME_ICON("export-file"));
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void TreeTableExportDialog::slotBrowse()
{
    FileDialog::Options options;
    options = QFlag(FileDialog::DontConfirmOverwrite);

    std::string selectedFilter;

    std::string fileName =
        FileDialog::getSaveFileName(app_,
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
    std::string path = fileNameLineEdit_->text();

    if (path.isEmpty())
    {
        (void)MessageBox::information(this,
                                       tr("Export File"),
                                       tr("Please choose a file name."));
        return;
    }

    if (File::exists(path.toStdString()))
    {
        MessageBox::StandardButton reply;

        reply = MessageBox::question(this,
                                      tr("Export File"),
                                      tr("Overwrite existing file?"),
                                      MessageBox::Yes | MessageBox::No);

        if (reply != MessageBox::Yes)
        {
            return;
        }
    }

    close();
    setResult(Dialog::Accepted);
}

void TreeTableExportDialog::slotReject()
{
    close();
    setResult(Dialog::Rejected);
}

std::shared_ptr<FileFormatInterface> TreeTableExportDialog::writer() const
{
    std::string path = fileNameLineEdit_->text().toStdString();
    std::string ext = toLower(File::fileExtension(path));

    if (ext == "csv")
    {
        std::shared_ptr<FileFormatCsv> csv = std::make_shared<FileFormatCsv>();
        csv->setFileName(path);
        return csv;
    }
    else
    {
        THROW("The selected file format is not supported. "
              "Please choose a different format.");
    }

    return std::shared_ptr<FileFormatInterface>();
}
