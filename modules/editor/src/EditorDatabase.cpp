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

/** @file EditorDatabase.cpp */

#include <EditorDatabase.hpp>
#include <FileIndexBuilder.hpp>
#include <Log.hpp>

static const char *EDITOR_BASE_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_BASE_KEY_DATA_SET = "datasets";
static const char *EDITOR_BASE_KEY_LAYER = "layers";
static const char *EDITOR_BASE_KEY_SETTINGS = "settings";
static const char *EDITOR_BASE_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_BASE_KEY_CLIP_FILTER = "clipFilter";

EditorDatabase::EditorDatabase()
{
    newProject();
    viewportsResize(1);
}

EditorDatabase::~EditorDatabase()
{
}

void EditorDatabase::newProject()
{
    path_ = File::currentPath() + "\\untitled.json";
    projectName_ = "Untitled";

    datasets_.clear();
    layers_.setDefault();
    classifications_.clear();
    viewports_.clearContent();

    unsavedChanges_ = false;
}

void EditorDatabase::openProject(const std::string &path)
{
    newProject();

    Json in;
    in.read(path);

    if (!in.isObject())
    {
        THROW("Project file '" + path + "' is not in JSON object");
    }

    try
    {
        path_ = path;

        // Project name
        if (in.contains(EDITOR_BASE_KEY_PROJECT_NAME))
        {
            projectName_ = in[EDITOR_BASE_KEY_PROJECT_NAME].string();
        }

        // Data sets
        if (in.contains(EDITOR_BASE_KEY_DATA_SET))
        {
            datasets_.read(in[EDITOR_BASE_KEY_DATA_SET], path_);
        }

        // Layers
        if (in.contains(EDITOR_BASE_KEY_LAYER))
        {
            layers_.read(in[EDITOR_BASE_KEY_LAYER]);
        }

        // Classifications
        if (in.contains(EDITOR_BASE_KEY_CLASSIFICATIONS))
        {
            classifications_.read(in[EDITOR_BASE_KEY_CLASSIFICATIONS]);
        }

        // Settings
        if (in.contains(EDITOR_BASE_KEY_SETTINGS))
        {
            settings_.read(in[EDITOR_BASE_KEY_SETTINGS]);
        }

        // Clip filter
        // if (in.contains(EDITOR_BASE_KEY_CLIP_FILTER))
        // {
        //     clipFilter_.read(in[EDITOR_BASE_KEY_CLIP_FILTER]);
        // }
        // else
        // {
        //     clipFilter_.clear();
        // }
    }
    catch (std::exception &e)
    {
        newProject();
        throw;
    }

    updateAfterRead();
}

void EditorDatabase::saveProject(const std::string &path)
{
    Json out;

    // Project name
    out[EDITOR_BASE_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    datasets_.write(out[EDITOR_BASE_KEY_DATA_SET]);

    // Layers
    layers_.write(out[EDITOR_BASE_KEY_LAYER]);

    // Classifications
    classifications_.write(out[EDITOR_BASE_KEY_CLASSIFICATIONS]);

    // Settings
    settings_.write(out[EDITOR_BASE_KEY_SETTINGS]);

    // Clip filter
    // clipFilter_.write(out[EDITOR_BASE_KEY_CLIP_FILTER]);

    out.write(path);

    unsavedChanges_ = false;
}

void EditorDatabase::openDataset(const std::string &path,
                                 const EditorSettingsImport &settings)
{
    try
    {
        datasets_.read(path, path_, settings, datasets_.boundary());
    }
    catch (std::exception &e)
    {
        throw;
    }

    updateAfterRead();

    unsavedChanges_ = true;
}

void EditorDatabase::setClassifications(
    const EditorClassifications &classifications)
{
    classifications_ = classifications;
    // viewClearRendered();
    unsavedChanges_ = true;
}

void EditorDatabase::setClipFilter(const ClipFilter &clipFilter)
{
    clipFilter_ = clipFilter;
    clipFilter_.boxView.setPercent(datasets_.boundary(),
                                   datasets_.boundary(),
                                   clipFilter_.box);

    viewports().selectBox(clipBoundary());

    // unsavedChanges_ = true;
}

void EditorDatabase::resetClipFilter()
{
    clipFilter_.box = datasets_.boundary();
    setClipFilter(clipFilter_);
}

Box<double> EditorDatabase::clipBoundary() const
{
    if (clipFilter_.enabled)
    {
        return clipFilter_.box;
    }

    return datasets_.boundary();
}

void EditorDatabase::updateAfterRead()
{
    clipFilter_.box = datasets_.boundary();
    clipFilter_.boxView = clipFilter_.box;
}

void EditorDatabase::setDatasets(const EditorDatasets &datasets)
{
    datasets_ = datasets;
    unsavedChanges_ = true;
}

void EditorDatabase::setLayers(const EditorLayers &layers)
{
    layers_ = layers;
    unsavedChanges_ = true;
}

void EditorDatabase::setSettingsView(const EditorSettingsView &settings)
{
    settings_.setView(settings);
    viewports_.setStateRender();
    unsavedChanges_ = true;
}

void EditorDatabase::addFilter(EditorFilter *filter)
{
    filters_.push_back(filter);
}

void EditorDatabase::applyFilters(EditorPage *page)
{
    for (auto &it : filters_)
    {
        /** @todo Collect enabled filters during preprocessing. */
        if (it->isFilterEnabled())
        {
            it->filterPage(page);
        }
    }
}

void EditorDatabase::viewportsResize(size_t n)
{
    viewports_.resize(this, n);
}
