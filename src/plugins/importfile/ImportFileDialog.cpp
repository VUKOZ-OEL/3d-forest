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

/** @file ImportFileDialog.cpp */

// Include 3D Forest.
#include <ImportFileDialog.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ImportFileDialog"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/importfile/", name))

ImportFileDialog::ImportFileDialog(MainWindow *mainWindow) : QDialog(mainWindow)
{
    // Widgets.
    QLabel *description = new QLabel(tr("Import action will modify "
                                        "the original input file."));

    description->setToolTip(tr("The points will be sorted by 3D spatial index "
                               "for fast access.\n"
                               "Some point data values will be normalized "
                               "(fixed) to match LAS specification.\n"
                               "Nonstandard application specific extra bytes "
                               "can be optionally removed from point data."));

    QGroupBox *options = new QGroupBox(tr("Options"));

    // Widgets with options.
    convertCheckBox_ = new QCheckBox;
    convertCheckBox_->setChecked(false);

    centerCheckBox_ = new QCheckBox;
    centerCheckBox_->setChecked(true);

    randomizeCheckBox_ = new QCheckBox;
    randomizeCheckBox_->setChecked(true);

    copyExtraBytesCheckBox_ = new QCheckBox;
    copyExtraBytesCheckBox_->setChecked(true);

    // Dialog buttons.
    acceptButton_ = new QPushButton(tr("Import"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    // Layout.
    QGridLayout *optionsLayout = new QGridLayout;
    int row = 0;
    optionsLayout->addWidget(new QLabel(tr("Convert to v1.4+")), row, 0);
    optionsLayout->addWidget(convertCheckBox_, row, 1);
    row++;
    optionsLayout->addWidget(new QLabel(tr("Center on screen")), row, 0);
    optionsLayout->addWidget(centerCheckBox_, row, 1);
    row++;
    optionsLayout->addWidget(new QLabel(tr("Randomize points")), row, 0);
    optionsLayout->addWidget(randomizeCheckBox_, row, 1);
    row++;
    optionsLayout->addWidget(new QLabel(tr("Copy extra bytes")), row, 0);
    optionsLayout->addWidget(copyExtraBytesCheckBox_, row, 1);
    row++;
    options->setLayout(optionsLayout);

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(options);
    dialogLayout->addSpacing(10);
    dialogLayout->addWidget(description);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    // Window.
    setWindowTitle(tr("Import File"));
    setWindowIcon(ICON("import_file"));
    setMaximumWidth(width());
    setMaximumHeight(height());
}

void ImportFileDialog::slotAccept()
{
    close();
    setResult(QDialog::Accepted);
}

void ImportFileDialog::slotReject()
{
    close();
    setResult(QDialog::Rejected);
}

SettingsImport ImportFileDialog::settings() const
{
    SettingsImport settingsImport;

    settingsImport.convertToVersion1Dot4 = convertCheckBox_->isChecked();
    settingsImport.centerPointsOnScreen = centerCheckBox_->isChecked();
    settingsImport.randomizePoints = randomizeCheckBox_->isChecked();
    settingsImport.copyExtraBytes = copyExtraBytesCheckBox_->isChecked();

    return settingsImport;
}
