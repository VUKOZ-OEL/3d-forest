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

/** @file FindVisibleObjects.cpp */

// Include 3D Forest.
#include <FindVisibleObjects.hpp>
#include <Application.hpp>
#include <ProgressDialog.hpp>

// Include local.
#define LOG_MODULE_NAME "FindVisibleObjects"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void FindVisibleObjects::run(std::unordered_set<size_t> &result,
                             Application *app)
{
    LOG_DEBUG(<< "Start searching for visible objects.");

    result.clear();

    // Editor.
    app->suspendThreads();

    Editor *editor = &app->editor();
    const Segments &segments = editor->segments();

    // Run.
    ProgressDialog progress(app);
    progress.setWindowTitle(tr("Processing..."));
    progress.setWindowModality(Ui::WindowModal);
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
            result.insert(segmentId);
        }

        counter++;
        if (counter == bulk)
        {
            counter = 0;

            app->processEvents();

            if (progress.wasCanceled())
            {
                break;
            }
        }
    }

    progress.close();

    LOG_DEBUG(<< "Finished searching for visible objects.");
}
