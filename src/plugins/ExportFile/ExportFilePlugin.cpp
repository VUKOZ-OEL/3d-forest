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

// Include 3D Forest.
#include <Application.hpp>
#include <ExportFileAction.hpp>
#include <ExportFileDialog.hpp>
#include <ExportFilePlugin.hpp>
#include <MessageBox.hpp>
#include <ProgressActionDialog.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "ExportFilePlugin"
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/exportfile/", name))

ExportFilePlugin::ExportFilePlugin() : app_(nullptr)
{
}

void ExportFilePlugin::initialize(Application *app)
{
    app_ = app;

    app_->createAction(
        &exportFileAction_,
        "File",
        "File Import/Export",
        tr("Export..."),
        tr("Export point cloud"),
        ICON("export-file"),
        [this]() { slotExportFile(); },
        MAIN_WINDOW_MENU_FILE_PRIORITY,
        60);
}

void ExportFilePlugin::slotExportFile()
{
    app_->suspendThreads();

    try
    {
        ExportFileDialog dialog(app_, fileName_);

        if (dialog.exec() == Dialog::Accepted)
        {
            std::shared_ptr<ExportFileFormatInterface> writer = dialog.writer();
            ExportFileProperties properties = dialog.properties();

            ExportFileAction exportFile(&app_->editor());
            exportFile.initialize(writer, properties);

            ProgressActionDialog::run(app_, "Exporting file", &exportFile);

            fileName_ = properties.fileName();
        }
    }
    catch (std::exception &e)
    {
        app_->showError(e.what());
    }
    catch (...)
    {
        app_->showError("Unknown error");
    }

    app_->resumeThreads();
}
