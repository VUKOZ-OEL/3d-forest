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

/** @file ExportPlugin.cpp */

#include <Log.hpp>

#include <ExportPlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QFileDialog>
#include <QMessageBox>

#define EXPORT_PLUGIN_FILTER "LAS (LASer) File (*.las)"
#define ICON(name) (ThemeIcon(":/export/", name))

ExportPlugin::ExportPlugin() : mainWindow_(nullptr)
{
}

void ExportPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&exportFile_,
                              "File",
                              "File Import/Export",
                              tr("Export As..."),
                              tr("Export point cloud dataset"),
                              ICON("export"),
                              this,
                              SLOT(slotExportFile()));
}

void ExportPlugin::slotExportFile()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(mainWindow_,
                                            tr("Save File As"),
                                            "",
                                            tr(EXPORT_PLUGIN_FILTER));

    if (fileName.isEmpty())
    {
        return;
    }

    // (void)projectSave(fileName);
}
