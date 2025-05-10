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

/** @file TreeTableSetManagementStatus.cpp */

// Include 3D Forest.
#include <InputComboBoxDialog.hpp>
#include <MainWindow.hpp>
#include <TreeTableSetManagementStatus.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "TreeTableSetManagementStatus"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void TreeTableSetManagementStatus::run(MainWindow *mainWindow,
                                       std::unordered_set<size_t> idList)
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
    for (const auto &id : idList)
    {
        size_t index = segments.index(id, false);

        if (index != SIZE_MAX)
        {
            segments[index].managementStatusId = newManagementStatusId;
        }
    }

    editor->setSegments(segments);
    mainWindow->update({Editor::TYPE_SEGMENT, Editor::TYPE_MANAGEMENT_STATUS});

    LOG_DEBUG(<< "Finished setting management status values.");
}
