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

#include <ExportFileDialog.hpp>
#include <MainWindow.hpp>
#include <Util.hpp>

#include <ExportFileCsv.hpp>
#include <ExportFileLas.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ExportFileDialog::ExportFileDialog(MainWindow *mainWindow,
                                   const QString &fileName)
    : QDialog(mainWindow),
      mainWindow_(mainWindow)
{
    // File name
    fileNameLineEdit_ = new QLineEdit;
    fileNameLineEdit_->setText(fileName);

    browseButton_ = new QPushButton(tr("Browse"));
    connect(browseButton_, SIGNAL(clicked()), this, SLOT(slotBrowse()));

    QHBoxLayout *fileNameLayout = new QHBoxLayout;
    fileNameLayout->addWidget(new QLabel(tr("File")));
    fileNameLayout->addWidget(fileNameLineEdit_);
    fileNameLayout->addWidget(browseButton_);

    // Attributes
    attributeCheckBox_.resize(5);

    attributeCheckBox_[0] = new QCheckBox(tr("XYZ coordinates"));
    attributeCheckBox_[0]->setChecked(true);
    attributeCheckBox_[0]->setEnabled(false);

    attributeCheckBox_[1] = new QCheckBox(tr("Intensity"));
    attributeCheckBox_[1]->setChecked(true);

    attributeCheckBox_[2] = new QCheckBox(tr("Classification"));
    attributeCheckBox_[2]->setChecked(true);

    attributeCheckBox_[3] = new QCheckBox(tr("Color"));
    attributeCheckBox_[3]->setChecked(true);

    attributeCheckBox_[4] = new QCheckBox(tr("Layer"));
    attributeCheckBox_[4]->setChecked(true);

    QVBoxLayout *attributeVBoxLayout = new QVBoxLayout;
    for (size_t i = 0; i < attributeCheckBox_.size(); i++)
    {
        attributeVBoxLayout->addWidget(attributeCheckBox_[i]);
    }

    QGroupBox *attributeGroupBox = new QGroupBox(tr("Point attributes"));
    attributeGroupBox->setLayout(attributeVBoxLayout);

    // Scale
    scaleComboBox_ = new QComboBox;
    scaleComboBox_->addItem("0.0001");
    scaleComboBox_->addItem("0.001");
    scaleComboBox_->addItem("0.01");
    scaleComboBox_->addItem("0.1");
    scaleComboBox_->addItem("1.0");
    scaleComboBox_->setCurrentText("0.001");

    QGridLayout *valueGridLayout = new QGridLayout;
    valueGridLayout->addWidget(new QLabel(tr("Scale")), 0, 0);
    valueGridLayout->addWidget(scaleComboBox_, 0, 1);

    // Buttons
    acceptButton_ = new QPushButton(tr("Export"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog layout
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(fileNameLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addWidget(attributeGroupBox);
    dialogLayout->addLayout(valueGridLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window
    setWindowTitle("Export File Settings");
    setMaximumWidth(600);
    setMaximumHeight(height());
}

void ExportFileDialog::slotBrowse()
{
    QFileDialog::Options options;
    options = QFlag(QFileDialog::DontConfirmOverwrite);

    QString selectedFilter;

    QString fileName =
        QFileDialog::getSaveFileName(mainWindow_,
                                     tr("Export File As"),
                                     fileNameLineEdit_->text(),
                                     tr("LAS (LASer) File (*.las);;"
                                        "Comma Separated Values (*.csv)"),
                                     &selectedFilter,
                                     options);

    if (fileName.isEmpty())
    {
        return;
    }

    fileNameLineEdit_->setText(fileName);
}

void ExportFileDialog::slotAccept()
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

void ExportFileDialog::slotReject()
{
    close();
    setResult(QDialog::Rejected);
}

std::shared_ptr<ExportFileInterface> ExportFileDialog::writer() const
{
    std::shared_ptr<ExportFileInterface> result;

    std::string path = fileNameLineEdit_->text().toStdString();
    std::string ext = toLower(File::fileExtension(path));

    if (ext == "csv")
    {
        result = std::make_shared<ExportFileCsv>();
    }
    else
    {
        result = std::make_shared<ExportFileLas>();
    }

    return result;
}

ExportFileProperties ExportFileDialog::properties() const
{
    ExportFileProperties result;

    // File name
    result.setFileName(fileNameLineEdit_->text().toStdString());

    // Point format
    uint32_t fmt = 0;

    for (size_t i = 0; i < attributeCheckBox_.size(); i++)
    {
        if (attributeCheckBox_[i]->isChecked())
        {
            fmt |= 1U << (i + 1);
        }
    }

    result.setFormat(fmt);

    // Scale
    result.setScale(scaleComboBox_->currentText().toDouble());

    return result;
}
