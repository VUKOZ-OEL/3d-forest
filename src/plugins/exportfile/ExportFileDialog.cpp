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

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ExportFileDialog::ExportFileDialog(MainWindow *mainWindow) : QDialog(mainWindow)
{
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
    dialogLayout->addWidget(attributeGroupBox);
    dialogLayout->addLayout(valueGridLayout);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window
    setWindowTitle("Export File Settings");
    setMaximumWidth(width());
    setMaximumHeight(height());
}

void ExportFileDialog::slotAccept()
{
    close();
    setResult(QDialog::Accepted);
}

void ExportFileDialog::slotReject()
{
    close();
    setResult(QDialog::Rejected);
}

ExportFileProperties ExportFileDialog::properties() const
{
    ExportFileProperties result;

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
