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

/** @file GuiPluginImport.cpp */

#include <FileLasIndexBuilder.hpp>
#include <Log.hpp>

#include <GuiPluginImport.hpp>
#include <GuiPluginImportDialog.hpp>
#include <GuiWindowMain.hpp>

#include <QCoreApplication>
#include <QFileDialog>
#include <QProgressBar>
#include <QProgressDialog>

#define GUI_PLUGIN_IMPORT_FILTER "LAS (LASer) File (*.las)"

#define ICON(name) (QIcon(":/import/" name GUI_ICON_THEME ".png"))

GuiPluginImport::GuiPluginImport(GuiWindowMain *window)
    : QObject(window),
      window_(window)
{
    window_->createAction(&actionImport_,
                          "File",
                          "File Import/Export",
                          tr("Import"),
                          tr("Import new dataset"),
                          ICON("add"),
                          this,
                          SLOT(slotImport()));

    window_->hideToolBar("File Import/Export");
}

void GuiPluginImport::slotImport()
{
    GuiPluginImport::import(window_);
}

static void guiPluginImport(GuiWindowMain *window);

static void guiPluginImportFile(const QString &path, GuiWindowMain *window);

static bool guiPluginImportCreateIndex(const QString &path,
                                       const EditorSettingsImport &settings,
                                       GuiWindowMain *window);

void GuiPluginImport::import(GuiWindowMain *window)
{
    try
    {
        guiPluginImport(window);
    }
    catch (std::exception &e)
    {
        window->showError(e.what());
        return;
    }
}

static void guiPluginImport(GuiWindowMain *window)
{
    QFileDialog dialog(window, QObject::tr("Import File"));
    dialog.setNameFilter(QObject::tr(GUI_PLUGIN_IMPORT_FILTER));

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

    guiPluginImportFile(fileName, window);
}

static void guiPluginImportFile(const QString &path, GuiWindowMain *window)
{
    window->suspendThreads();

    GuiPluginImportDialog dialog(window);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    EditorSettingsImport settings = dialog.getSettings();

    if (guiPluginImportCreateIndex(path, settings, window))
    {
        window->editor().open(path.toStdString(), settings);
    }

    window->updateEverything();
}

static bool guiPluginImportCreateIndex(const QString &path,
                                       const EditorSettingsImport &settings,
                                       GuiWindowMain *window)
{
    // If the index already exists, then return success.
    std::string pathStd;
    std::string pathFile;
    std::string pathIndex;

    pathStd = path.toStdString();
    pathFile = File::resolvePath(pathStd, window->editor().projectPath());
    pathIndex = FileLasIndexBuilder::extension(pathFile);

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
    FileLasIndexBuilder builder;
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