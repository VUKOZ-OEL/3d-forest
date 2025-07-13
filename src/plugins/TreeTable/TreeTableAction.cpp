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

/** @file TreeTableAction.cpp */

// Include 3D Forest.
#include <MainWindow.hpp>
#include <TreeTableAction.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeTableAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void TreeTableAction::showTrees(MainWindow *mainWindow,
                                std::unordered_set<size_t> idList)
{
    LOG_DEBUG(<< "Start showing trees.");

    QueryFilterSet filter = mainWindow->editor().segmentsFilter();

    for (const auto &id : idList)
    {
        filter.setEnabled(id, true);
    }

    mainWindow->editor().setSegmentsFilter(filter);
    mainWindow->updateFilter();

    LOG_DEBUG(<< "Finished showing trees.");
}

void TreeTableAction::hideTrees(MainWindow *mainWindow,
                                std::unordered_set<size_t> idList)
{
    LOG_DEBUG(<< "Start showing trees.");

    QueryFilterSet filter = mainWindow->editor().segmentsFilter();

    for (const auto &id : idList)
    {
        filter.setEnabled(id, false);
    }

    mainWindow->editor().setSegmentsFilter(filter);
    mainWindow->updateFilter();

    LOG_DEBUG(<< "Finished showing trees.");
}
