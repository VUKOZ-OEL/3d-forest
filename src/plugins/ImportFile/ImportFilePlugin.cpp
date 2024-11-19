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

/** @file ImportFilePlugin.cpp */

// Include 3D Forest.
#include <ImportFileDialog.hpp>
#include <ImportFilePlugin.hpp>
#include <IndexFileBuilder.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QFileDialog>
#include <QProgressBar>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "ImportFilePlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define IMPORT_PLUGIN_FILTER "LAS (LASer) File (*.las)"
#define ICON(name) (ThemeIcon(":/ImportFileResources/", name))

ImportFilePlugin::ImportFilePlugin() : mainWindow_(nullptr)
{
}

void ImportFilePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&importFileAction_,
                              "File",
                              "File Import/Export",
                              tr("Import..."),
                              tr("Import new point cloud dataset"),
                              ICON("import-file"),
                              this,
                              SLOT(slotImportFile()),
                              MAIN_WINDOW_MENU_FILE_PRIORITY,
                              50);

    mainWindow_->hideToolBar("File Import/Export");
}

void ImportFilePlugin::slotImportFile()
{
    importFile();
}

static void importPluginDialog(MainWindow *mainWindow);

static void importPluginFile(const QString &path,
                             const ImportSettings &settings,
                             MainWindow *mainWindow);

static bool importPluginCreateIndex(const QString &path,
                                    const ImportSettings &settings,
                                    MainWindow *mainWindow);

void ImportFilePlugin::importFile()
{
    try
    {
        importPluginDialog(mainWindow_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
        return;
    }
}

static void importPluginDialog(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start importing files.");

    QFileDialog fileDialog(mainWindow, QObject::tr("Import File"));
    fileDialog.setNameFilter(QObject::tr(IMPORT_PLUGIN_FILTER));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    if (fileDialog.exec() == QDialog::Rejected)
    {
        LOG_DEBUG(<< "Canceled importing files from the dialog.");
        return;
    }

    QStringList files = fileDialog.selectedFiles();
    QStringList selectedFiles;
    for (auto const &file : files)
    {
        if (file.length() > 0)
        {
            selectedFiles.append(file);
        }
    }

    LOG_DEBUG(<< "Selected <" << selectedFiles.count() << "> files.");
    if (selectedFiles.count() < 1)
    {
        LOG_DEBUG(<< "Canceled importing files. No files selected.");
        return;
    }

    // Stop rendering.
    mainWindow->suspendThreads();

    // Import settings.
    ImportFileDialog settingsDialog(mainWindow);

    if (settingsDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    ImportSettings settings = settingsDialog.settings();

    // Import.
    for (auto const &file : selectedFiles)
    {
        importPluginFile(file, settings, mainWindow);
    }

    // Update.
    mainWindow->updateNewProject();
    mainWindow->slotRenderViewports();

    LOG_DEBUG(<< "Finished importing files.");
}

static void importPluginFile(const QString &path,
                             const ImportSettings &settings,
                             MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Import file <" << path.toStdString() << ">.");

    if (importPluginCreateIndex(path, settings, mainWindow))
    {
        mainWindow->editor().open(path.toStdString(), settings);
    }
}

static bool importPluginCreateIndex(const QString &path,
                                    const ImportSettings &settings,
                                    MainWindow *mainWindow)
{
    // If the index already exists, then return success.
    std::string pathStd;
    std::string pathFile;
    std::string pathIndex;

    pathStd = path.toStdString();
    pathFile = File::resolvePath(pathStd, mainWindow->editor().projectPath());
    pathIndex = IndexFileBuilder::extension(pathFile);

    if (File::exists(pathIndex))
    {
        return true;
    }

    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    QProgressDialog progressDialog(mainWindow);
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
    IndexFileBuilder builder;
    builder.start(pathStd, pathStd, settings);

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