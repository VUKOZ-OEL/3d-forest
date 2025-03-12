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
#include <InfoDialog.hpp>
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

#define ICON(name) (ThemeIcon(":/ImportFileResources/", name))

ImportFileDialog::ImportFileDialog(MainWindow *mainWindow)
    : QDialog(mainWindow),
      helpDialog_(nullptr)
{
    int row = 0;

    // Dataset options.
    QGroupBox *datasetOptions = new QGroupBox(tr("Options"));

    importFilesAsSeparateTreesCheckBox_ = new QCheckBox;
    importFilesAsSeparateTreesCheckBox_->setChecked(false);

    translateToOriginCheckBox_ = new QCheckBox;
    translateToOriginCheckBox_->setChecked(false);

    QGridLayout *ldo = new QGridLayout;
    row = 0;
    ldo->addWidget(new QLabel(tr("Import files as separate trees")), row, 0);
    ldo->addWidget(importFilesAsSeparateTreesCheckBox_, row, 1);
    row++;
    ldo->addWidget(new QLabel(tr("Translate to the origin")), row, 0);
    ldo->addWidget(translateToOriginCheckBox_, row, 1);
    row++;

    datasetOptions->setLayout(ldo);

    // File format options.
    QGroupBox *fileFormatOptions = new QGroupBox(tr("File format options"));

    convertToVersion1Dot4CheckBox_ = new QCheckBox;
    convertToVersion1Dot4CheckBox_->setChecked(false);

    randomizePointsCheckBox_ = new QCheckBox;
    randomizePointsCheckBox_->setChecked(true);

    copyExtraBytesCheckBox_ = new QCheckBox;
    copyExtraBytesCheckBox_->setChecked(true);

    QGridLayout *lfo = new QGridLayout;
    row = 0;
    lfo->addWidget(new QLabel(tr("Convert to v1.4+")), row, 0);
    lfo->addWidget(convertToVersion1Dot4CheckBox_, row, 1);
    row++;
    lfo->addWidget(new QLabel(tr("Randomize points")), row, 0);
    lfo->addWidget(randomizePointsCheckBox_, row, 1);
    row++;
    lfo->addWidget(new QLabel(tr("Copy extra bytes")), row, 0);
    lfo->addWidget(copyExtraBytesCheckBox_, row, 1);
    row++;

    fileFormatOptions->setLayout(lfo);

    // Description.
    QLabel *description = new QLabel(tr("Import action will modify "
                                        "the original input file."));

    description->setToolTip(tr("The points will be sorted by 3D spatial index "
                               "for fast access.\n"
                               "Some point data values will be normalized "
                               "(fixed) to match LAS specification.\n"
                               "Nonstandard application specific extra bytes "
                               "can be optionally removed from point data."));

    // Dialog buttons.
    helpButton_ = new QPushButton(tr("Help"));
    helpButton_->setIcon(THEME_ICON("question"));
    connect(helpButton_, SIGNAL(clicked()), this, SLOT(slotHelp()));

    acceptButton_ = new QPushButton(tr("Import"));
    acceptButton_->setIcon(THEME_ICON("run"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(slotAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(slotReject()));

    QHBoxLayout *dialogButtons = new QHBoxLayout;
    dialogButtons->addWidget(helpButton_);
    dialogButtons->addStretch();
    dialogButtons->addWidget(acceptButton_);
    dialogButtons->addWidget(rejectButton_);

    // Dialog.
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(datasetOptions);
    dialogLayout->addWidget(fileFormatOptions);
    dialogLayout->addSpacing(10);
    dialogLayout->addWidget(description);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(dialogButtons);
    dialogLayout->addStretch();

    setLayout(dialogLayout);

    setWindowTitle(tr("Import File"));
    setWindowIcon(ICON("import-file"));
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

ImportSettings ImportFileDialog::settings() const
{
    ImportSettings settings;

    settings.importFilesAsSeparateTrees =
        importFilesAsSeparateTreesCheckBox_->isChecked();
    settings.translateToOrigin = translateToOriginCheckBox_->isChecked();

    settings.convertToVersion1Dot4 =
        convertToVersion1Dot4CheckBox_->isChecked();
    settings.randomizePoints = randomizePointsCheckBox_->isChecked();
    settings.copyExtraBytes = copyExtraBytesCheckBox_->isChecked();

    return settings;
}

void ImportFileDialog::slotHelp()
{
    QString t;
    t = "<h3>Import File</h3>"
        "<h2>Options</h2>"
        "The import file dialog allows to set the following options: "
        "<ul>"
        "<li><b>Import files as separate trees</b> -"
        " Import each file as a separate tree.</li>"
        "<li><b>Translate to the origin</b> -"
        " The coordinates will be translated to the origin of the coordinate"
        " system.</li>"
        "<li><b>Convert to v1.4+</b> -"
        " Convert LAS file to version 1.4 if it is in lower version."
        " Version 1.4 allows to use more classifications, GPS coordinates,"
        " etc.</li>"
        "<li><b>Randomize points</b> -"
        " It is suggested to randomize the order of points in LAS files"
        " to prevent eye popping artifacts caused displaying subsets"
        " of points by level of details. It is possible to uncheck this "
        " option if a file was already randomized.</li>"
        "<li><b>Copy extra bytes</b> -"
        " If this option is checked, then the import process preservers"
        " all extra bytes in each point which are stored beyond the size"
        " specified by LAS specification.</li>"
        "</ul>"
        "<h2>Modifications of the Original Input File</h2>"
        "<ol>"
        "<li>The points will be sorted by 3D spatial index"
        " for fast access.</li>"
        "<li>Some point data values will be normalized "
        " (fixed) to match LAS specification.</li>"
        "<li>Nonstandard application specific extra bytes "
        " can be optionally removed from point data.</li>"
        "</ol>";

    if (!helpDialog_)
    {
        helpDialog_ = new InfoDialog(this, 550, 450);
        helpDialog_->setWindowTitle(tr("Import File Help"));
        helpDialog_->setText(t);
    }

    helpDialog_->show();
    helpDialog_->raise();
    helpDialog_->activateWindow();
}