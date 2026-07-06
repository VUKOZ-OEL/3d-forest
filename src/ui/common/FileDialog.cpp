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

/** @file FileDialog.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <FileDialog.hpp>

// Include local.
#define LOG_MODULE_NAME "FileDialog"
#include <Log.hpp>

std::vector<std::string> FileDialog::selectFiles(Application *app,
                                                const std::string &dialogTitle,
                                                const std::string &filter)
{
    std::vector<std::string> list;

#if 0
    QFileDialog fileDialog(app, QObject::tr("Import File"));
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
#endif

    return list;
}

std::string FileDialog::getOpenFileName(Application *app,
                                        const std::string &dialogTitle,
                                        const std::string &filter)
{
    return "";
}

std::string FileDialog::getSaveFileName(Application *app,
                                        const std::string &dialogTitle,
                                        const std::string &filter)
{
    return "";
}
