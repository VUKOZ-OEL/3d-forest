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

/** @file ExportFilePlugin.cpp */

#include <Log.hpp>

#include <ExportFile.hpp>
#include <ExportFileDialog.hpp>
#include <ExportFilePlugin.hpp>
#include <MainWindow.hpp>
#include <ProgressDialog.hpp>
#include <ThemeIcon.hpp>

#include <QMessageBox>

#define ICON(name) (ThemeIcon(":/exportfile/", name))

ExportFilePlugin::ExportFilePlugin() : mainWindow_(nullptr)
{
}

void ExportFilePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&exportFileAction_,
                              "File",
                              "File Import/Export",
                              tr("Export File As..."),
                              tr("Export point cloud"),
                              ICON("export_file"),
                              this,
                              SLOT(slotExportFile()));
}

void ExportFilePlugin::slotExportFile()
{
    mainWindow_->suspendThreads();

    try
    {
        ExportFileDialog dialog(mainWindow_, fileName_);

        if (dialog.exec() == QDialog::Accepted)
        {
            ExportFile exportFile(&mainWindow_->editor());
            exportFile.initialize(dialog.writer(), dialog.properties());
            fileName_ = QString::fromStdString(dialog.properties().fileName());

            ProgressDialog::run(mainWindow_, "Exporting file", &exportFile);
        }
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
    }
    catch (...)
    {
        mainWindow_->showError("Unknown error");
    }

    mainWindow_->resumeThreads();
}
