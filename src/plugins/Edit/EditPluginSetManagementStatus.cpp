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

/** @file EditPluginSetManagementStatus.cpp */

// Include 3D Forest.
#include <EditPluginSetManagementStatus.hpp>
#include <InputComboBoxDialog.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "EditPluginSetManagementStatus"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void EditPluginSetManagementStatus::run(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start setting management status values.");

    size_t newManagementStatusId = 0;

    // Editor.
    mainWindow->suspendThreads();

    Editor *editor = &mainWindow->editor();

    // Input.
    InputComboBoxDialog dialog;
    dialog.setWindowTitle("Select a management status");

    // Set dialog items.
    const ManagementStatusList &statusList = editor->managementStatusList();
    for (size_t i = 0; i < statusList.size(); i++)
    {
        const ManagementStatus &status = statusList[i];
        QString text = QString::number(status.id) + " : " +
                       QString::fromStdString(status.label);
        dialog.addItem(text);
    }

    // Open the dialog.
    if (dialog.exec() == QDialog::Accepted)
    {
        // Run.
        size_t index = static_cast<size_t>(dialog.currentIndex());
        newManagementStatusId = statusList[index].id;
        LOG_DEBUG(<< "User entered index <" << dialog.currentIndex() << "> id <"
                  << newManagementStatusId << ">.");
    }
    else
    {
        // Cancel.
        LOG_DEBUG(<< "Canceled setting management status values.");
        return;
    }

    // Edit segments.
    Segments segments = editor->segments();

    // Run.
    QProgressDialog progress("Processing...", "Cancel", 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    Query query(editor);
    query.setWhere(editor->viewports().where());
    query.exec();

    int bulk = 1000;
    int counter = 0;
    while (query.next())
    {
        size_t segmentId = query.segment();
        size_t segmentIndex = segments.index(segmentId, false);

        if (segmentIndex != SIZE_MAX)
        {
            segments[segmentIndex].managementStatusId = newManagementStatusId;
        }

        counter++;
        if (counter == bulk)
        {
            counter = 0;

            QCoreApplication::processEvents();

            if (progress.wasCanceled())
            {
                break;
            }
        }
    }

    progress.close();

    editor->setSegments(segments);
    mainWindow->update({Editor::TYPE_SEGMENT, Editor::TYPE_MANAGEMENT_STATUS});

    LOG_DEBUG(<< "Finished setting management status values.");
}
