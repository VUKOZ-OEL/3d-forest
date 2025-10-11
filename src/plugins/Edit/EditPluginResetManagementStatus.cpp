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

/** @file EditPluginResetManagementStatus.cpp */

// Include 3D Forest.
#include <EditPluginResetManagementStatus.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QProgressDialog>

// Include local.
#define LOG_MODULE_NAME "EditPluginResetManagementStatus"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void EditPluginResetManagementStatus::run(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start resetting management status values.");

    size_t newManagementStatusId = 0;

    // Editor.
    mainWindow->suspendThreads();

    Editor *editor = &mainWindow->editor();

    // Edit segments.
    Segments segments = editor->segments();
    for (size_t i = 0; i < segments.size(); i++)
    {
        segments[i].managementStatusId = newManagementStatusId;
    }

    editor->setSegments(segments);
    mainWindow->update({Editor::TYPE_SEGMENT, Editor::TYPE_MANAGEMENT_STATUS});

    LOG_DEBUG(<< "Finished resetting management status values.");
}
