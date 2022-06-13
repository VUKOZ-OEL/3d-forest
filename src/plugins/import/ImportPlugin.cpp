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

/** @file ImportPlugin.cpp */

#include <IndexFileBuilder.hpp>
#include <Log.hpp>

#include <ImportDialog.hpp>
#include <ImportPlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QCoreApplication>
#include <QFileDialog>
#include <QProgressBar>
#include <QProgressDialog>

#define IMPORT_PLUGIN_FILTER "LAS (LASer) File (*.las)"
//#define ICON(name) (ThemeIcon(":/import/", name))

ImportPlugin::ImportPlugin() : mainWindow_(nullptr)
{
}

void ImportPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&actionImport_,
                              "File",
                              "File Import/Export",
                              tr("Import"),
                              tr("Import new point cloud dataset"),
                              THEME_ICON("plus"),
                              this,
                              SLOT(slotImport()));

    mainWindow_->hideToolBar("File Import/Export");
}

void ImportPlugin::slotImport()
{
    ImportPlugin::import(mainWindow_);
}

static void importPluginDialog(MainWindow *mainWindow);

static void importPluginFile(const QString &path, MainWindow *mainWindow);

static bool importPluginCreateIndex(const QString &path,
                                    const SettingsImport &settings,
                                    MainWindow *mainWindow);

void ImportPlugin::import(MainWindow *mainWindow)
{
    try
    {
        importPluginDialog(mainWindow);
    }
    catch (std::exception &e)
    {
        mainWindow->showError(e.what());
        return;
    }
}

static void importPluginDialog(MainWindow *mainWindow)
{
    QFileDialog dialog(mainWindow, QObject::tr("Import File"));
    dialog.setNameFilter(QObject::tr(IMPORT_PLUGIN_FILTER));

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

    importPluginFile(fileName, mainWindow);
}

static void importPluginFile(const QString &path, MainWindow *mainWindow)
{
    mainWindow->suspendThreads();

    ImportDialog dialog(mainWindow);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    SettingsImport settings = dialog.getSettings();

    if (importPluginCreateIndex(path, settings, mainWindow))
    {
        mainWindow->editor().open(path.toStdString(), settings);
    }

    mainWindow->updateEverything();
}

static bool importPluginCreateIndex(const QString &path,
                                    const SettingsImport &settings,
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