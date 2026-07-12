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

/** @file ExportFileDialog.cpp */

// Include 3D Forest.
#include <ExportFileDialog.hpp>
#include <ExportFileFormatCsv.hpp>
#include <ExportFileFormatLas.hpp>
#include <Application.hpp>
#include <ThemeIcon.hpp>
#include <Util.hpp>
#include <CheckBox.hpp>
#include <ComboBox.hpp>
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
#define LOG_MODULE_NAME "ExportFileDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/exportfile/", name))

ExportFileDialog::ExportFileDialog(Application *app,
                                   const std::string &fileName)
    : Dialog(app),
      app_(app)
{
    // File name.
    fileNameLineEdit_ = new LineEdit;
    fileNameLineEdit_->setText(fileName);

    browseButton_ = new PushButton(tr("Browse"));
    browseButton_->clicked.connect([this]()
    {
        slotBrowse();
    });

    HBoxLayout *fileNameLayout = new HBoxLayout;
    fileNameLayout->addWidget(new Label(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Attributes.
    attributeCheckBox_.resize(5);

    attributeCheckBox_[0] = new CheckBox(tr("XYZ coordinates"));
    attributeCheckBox_[0]->setChecked(true);
    attributeCheckBox_[0]->setEnabled(false);

    attributeCheckBox_[1] = new CheckBox(tr("Intensity"));
    attributeCheckBox_[1]->setChecked(true);

    attributeCheckBox_[2] = new CheckBox(tr("Classification"));
    attributeCheckBox_[2]->setChecked(true);

    attributeCheckBox_[3] = new CheckBox(tr("Color"));
    attributeCheckBox_[3]->setChecked(true);

    attributeCheckBox_[4] = new CheckBox(tr("Segment"));
    attributeCheckBox_[4]->setChecked(true);

    VBoxLayout *attributeVBoxLayout = new VBoxLayout;
    for (size_t i = 0; i < attributeCheckBox_.size(); i++)
    {
        attributeVBoxLayout->addWidget(attributeCheckBox_[i]);
    }

    GroupBox *attributeGroupBox = new GroupBox(tr("Point attributes"));
    attributeGroupBox->setLayout(attributeVBoxLayout);

    // Other options.
    scaleComboBox_ = new ComboBox;
    scaleComboBox_->addItem("0.0001");
    scaleComboBox_->addItem("0.001");
    scaleComboBox_->addItem("0.01");
    scaleComboBox_->addItem("0.1");
    scaleComboBox_->addItem("1.0");
    scaleComboBox_->setCurrentText("0.001");

    filterEnabledCheckBox_ = new CheckBox;
    filterEnabledCheckBox_->setChecked(true);

    GridLayout *valueGridLayout = new GridLayout;
    valueGridLayout->addWidget(new Label(tr("Scale")), 0, 0);
    valueGridLayout->addWidget(scaleComboBox_, 0, 1);
    valueGridLayout->addWidget(new Label(tr("Use current filter")), 1, 0);
    valueGridLayout->addWidget(filterEnabledCheckBox_, 1, 1);

    // Buttons.
    acceptButton_ = new PushButton(tr("Export"));
    acceptButton_->clicked.connect([this]()
    {
        slotAccept();
    });

    rejectButton_ = new PushButton(tr("Cancel"));
    rejectButton_->clicked.connect([this]()
    {
        slotReject();
    });

    HBoxLayout *dialogButtons = new HBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog layout.
    VBoxLayout *dialogLayout = new VBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addWidget(attributeGroupBox);
    dialogLayout->addLayout(valueGridLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window.
    setWindowTitle(tr("Export File"));
    setWindowIcon(ICON("export-file"));
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void ExportFileDialog::slotBrowse()
{
    std::string selectedFilter;

    std::string fileName =
        FileDialog::getSaveFileName(app_,
                                     tr("Export File As"),
                                     fileNameLineEdit_->text(),
                                     tr("LAS (LASer) File (*.las);;"
                                        "Comma Separated Values (*.csv)"),
                                     &selectedFilter,
                                     FileDialog::DontConfirmOverwrite);

    if (fileName.empty())
    {
        return;
    }

    fileNameLineEdit_->setText(fileName);
}

void ExportFileDialog::slotAccept()
{
    std::string path = fileNameLineEdit_->text();

    if (path.empty())
    {
        (void)MessageBox::information(app_,
                                       tr("Export File"),
                                       tr("Please choose a file name."));
        return;
    }

    if (File::exists(path))
    {
        int reply;

        reply = MessageBox::question(app_,
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

void ExportFileDialog::slotReject()
{
    close();
    setResult(Dialog::Rejected);
}

std::shared_ptr<ExportFileFormatInterface> ExportFileDialog::writer() const
{
    std::shared_ptr<ExportFileFormatInterface> result;

    std::string path = fileNameLineEdit_->text();
    std::string ext = toLower(File::fileExtension(path));

    if (ext == "csv")
    {
        result = std::make_shared<ExportFileFormatCsv>();
    }
    else
    {
        result = std::make_shared<ExportFileFormatLas>();
    }

    return result;
}

ExportFileProperties ExportFileDialog::properties() const
{
    ExportFileProperties result;

    // File name.
    result.setFileName(fileNameLineEdit_->text());

    // Point format.
    uint32_t fmt = 0;

    for (size_t i = 0; i < attributeCheckBox_.size(); i++)
    {
        if (attributeCheckBox_[i]->isChecked())
        {
            fmt |= 1U << (i + 1);
        }
    }

    result.setFormat(fmt);

    // Scale.
    result.setScale(toDouble(scaleComboBox_->currentText()));

    // Filter.
    result.setFilterEnabled(filterEnabledCheckBox_->isChecked());

    return result;
}
