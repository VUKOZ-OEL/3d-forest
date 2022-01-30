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

/** @file WindowFileImport.cpp */

#include <FileIndexBuilder.hpp>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <WindowFileImport.hpp>
#include <WindowMain.hpp>

#define WINDOW_FILE_IMPORT_FILTER "LAS (LASer) File (*.las)"

// Import dialog
WindowFileImport::WindowFileImport(QWidget *parent) : QDialog(parent)
{
    // Widgets
    QLabel *description = new QLabel(tr("This operation may modify"
                                        " the input file."));

    description->setToolTip(tr("The file size may get bigger.\n"
                               "Unknown User data will be lost.\n"
                               "Some values will be normalized."));

    QGroupBox *options = new QGroupBox(tr("Options"));

    centerCheckBox_ = new QCheckBox;
    centerCheckBox_->setChecked(true);

    convertCheckBox_ = new QCheckBox;
    convertCheckBox_->setChecked(true);
    convertCheckBox_->setEnabled(false);

    acceptButton_ = new QPushButton(tr("Import"));
    connect(acceptButton_, SIGNAL(clicked()), this, SLOT(setResultAccept()));

    rejectButton_ = new QPushButton(tr("Cancel"));
    connect(rejectButton_, SIGNAL(clicked()), this, SLOT(setResultReject()));

    // Layout
    QGridLayout *optionsLayout = new QGridLayout;
    int row = 0;
    optionsLayout->addWidget(new QLabel(tr("Center offset")), row, 0);
    optionsLayout->addWidget(centerCheckBox_, row, 1);
    row++;
    optionsLayout->addWidget(new QLabel(tr("Convert to v1.4+")), row, 0);
    optionsLayout->addWidget(convertCheckBox_, row, 1);
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

    // Window
    setWindowTitle("Import File");
    setMaximumWidth(width());
    setMaximumHeight(height());
}

void WindowFileImport::setResultAccept()
{
    close();
    setResult(QDialog::Accepted);
}

void WindowFileImport::setResultReject()
{
    close();
    setResult(QDialog::Rejected);
}

EditorSettingsImport WindowFileImport::settings() const
{
    EditorSettingsImport settings;

    settings.setCenterEnabled(centerCheckBox_->isChecked());

    return settings;
}

// Import functionality
static void windowFileImport(WindowMain *window, Editor *editor);

static void windowFileImportFile(const QString &path,
                                 WindowMain *window,
                                 Editor *editor);

static bool windowFileImportCreateIndex(const QString &path,
                                        const EditorSettingsImport &settings,
                                        WindowMain *window,
                                        Editor *editor);

void WindowFileImport::import(WindowMain *window, Editor *editor)
{
    try
    {
        windowFileImport(window, editor);
    }
    catch (std::exception &e)
    {
        window->showError(e.what());
        return;
    }
}

static void windowFileImport(WindowMain *window, Editor *editor)
{
    QFileDialog dialog(window, QObject::tr("Import File"));
    dialog.setNameFilter(QObject::tr(WINDOW_FILE_IMPORT_FILTER));

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    QStringList files = dialog.selectedFiles();
    if (files.count() < 1)
    {
        return;
    }

    QString fileName = files.at(0);
    if (fileName.isEmpty())
    {
        return;
    }

    windowFileImportFile(fileName, window, editor);
}

static void windowFileImportFile(const QString &path,
                                 WindowMain *window,
                                 Editor *editor)
{
    editor->cancelThreads();

    WindowFileImport dialog(window);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    EditorSettingsImport settings = dialog.settings();

    if (windowFileImportCreateIndex(path, settings, window, editor))
    {
        editor->open(path.toStdString(), settings);
    }

    window->updateProject();
}

static bool windowFileImportCreateIndex(const QString &path,
                                        const EditorSettingsImport &settings,
                                        WindowMain *window,
                                        Editor *editor)
{
    // If the index already exists, then return success.
    const std::string pathStd = path.toStdString();
    std::string pathFile = File::resolvePath(pathStd, editor->projectPath());
    std::string pathIndex = FileIndexBuilder::extension(pathFile);

    if (File::exists(pathIndex))
    {
        return true;
    }

    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    QProgressDialog progressDialog(window);
    progressDialog.setWindowTitle(QObject::tr("Create Index"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setMinimumDuration(0);

    QProgressBar *progressBar = new QProgressBar(&progressDialog);
    progressBar->setTextVisible(false);
    progressBar->setRange(0, 100);
    progressBar->setValue(progressBar->minimum());
    progressDialog.setBar(progressBar);

    // Initialize index builder.
    FileIndexBuilder builder;
    builder.start(pathStd, pathStd, settings.indexSettings());

    char buffer[80];

    progressDialog.show();

    // Do import operation in progress loop.
    while (!builder.end())
    {
        // Update progress.
        double value = builder.percent();

        std::snprintf(buffer,
                      sizeof(buffer),
                      "Overall progress: %6.2f %% complete",
                      value);

        progressDialog.setValue(static_cast<int>(value));
        progressDialog.setLabelText(buffer);

        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
        {
            return false;
        }

        // Process several bytes of the operation.
        builder.next();
    }

    progressDialog.setValue(progressDialog.maximum());

    return true;
}
