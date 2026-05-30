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
#include <OpenFileDialog.hpp>

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

void TreeTableAction::readMesh(MainWindow *mainWindow,
                               const std::unordered_set<size_t> &idList,
                               const std::string &meshName)
{
    LOG_DEBUG(<< "Start read mesh");

    if (idList.empty())
    {
        LOG_DEBUG(<< "No tree selected");
        return;
    }

    Editor *editor = &mainWindow->editor();
    Segments segments = editor->segments();

    size_t id = *idList.begin();
    size_t index = segments.index(id, false);

    if (index == SIZE_MAX)
    {
        LOG_DEBUG(<< "Tree not found");
        return;
    }

    std::string path = OpenFileDialog::dialog(mainWindow, "(*.ply)");
    if (path.empty())
    {
        LOG_DEBUG(<< "No file selected");
        return;
    }

    double ppm = editor->settings().unitsSettings().pointsPerMeter()[0];

    LOG_DEBUG(<< "Read mesh from file <" << path << ">");

    Mesh mesh;
    mesh.read(path, ppm);
    mesh.name = meshName;

    segments[index].meshList[mesh.name] = std::move(mesh);

    editor->setSegments(segments);

    LOG_DEBUG(<< "Finished read mesh");
}

void TreeTableAction::deleteMesh(MainWindow *mainWindow,
                                 const std::unordered_set<size_t> &idList,
                                 const std::string &meshName)
{
    LOG_DEBUG(<< "Delete mesh <" << meshName << ">");

    if (idList.empty())
    {
        LOG_DEBUG(<< "No tree selected");
        return;
    }

    Editor *editor = &mainWindow->editor();
    Segments segments = editor->segments();

    size_t id = *idList.begin();
    size_t index = segments.index(id, false);

    if (index == SIZE_MAX)
    {
        LOG_DEBUG(<< "Tree not found");
        return;
    }

    for (const auto &id : idList)
    {
        size_t index = segments.index(id, false);

        if (index != SIZE_MAX)
        {
            segments[index].meshList.erase(meshName);
        }
    }

    LOG_DEBUG(<< "Finished delete mesh");
}
