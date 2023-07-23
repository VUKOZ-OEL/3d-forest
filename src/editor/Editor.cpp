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

/** @file Editor.cpp */

#include <Editor.hpp>
#include <IndexFileBuilder.hpp>
#include <Util.hpp>

#define LOG_MODULE_NAME "Editor"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

static const char *EDITOR_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_KEY_DATA_SET = "datasets";
static const char *EDITOR_KEY_LAYER = "layers";
static const char *EDITOR_KEY_SETTINGS = "settings";
static const char *EDITOR_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_KEY_CLIP_FILTER = "clipFilter";
static const char *EDITOR_KEY_ELEVATION_RANGE = "elevationRange";

Editor::Editor()
{
    LOG_DEBUG(<< "Create.");
    close();
    viewportsResize(1);
}

Editor::~Editor()
{
    LOG_DEBUG(<< "Destroy.");
}

void Editor::close()
{
    LOG_DEBUG(<< "Close.");

    path_ = File::join(File::currentPath(), "untitled.json");
    projectName_ = "Untitled";

    datasets_.clear();
    datasetsFilter_.clear();
    datasetsFilter_.setFilterEnabled(true);

    layers_.setDefault();
    layersFilter_.clear();
    layersFilter_.setFilter(0, true);
    layersFilter_.setFilterEnabled(true);

    classifications_.clear();
    classificationsFilter_.clear();
    for (size_t i = 0; i < classifications_.size(); i++)
    {
        classificationsFilter_.setFilter(i, true);
    }
    classificationsFilter_.setFilterEnabled(true);

    viewports_.clearContent();

    clipFilter_.clear();
    elevationFilter_.clear();
    descriptorFilter_.set(0.0, 1.0);

    unsavedChanges_ = false;
}

void Editor::open(const std::string &path, const SettingsImport &settings)
{
    LOG_DEBUG(<< "Open path <" << path << ">.");

    // Get filename extension in lower case (no UTF).
    std::string ext = toLower(File::fileExtension(path));

    if (ext == "json")
    {
        // Open new project from json format.
        openProject(path);
    }
    else
    {
        // Add new dataset to existing project.
        openDataset(path, settings);
    }
}

void Editor::openProject(const std::string &path)
{
    LOG_DEBUG(<< "Open project path <" << path << ">.");

    close();

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
        if (in.contains(EDITOR_KEY_PROJECT_NAME))
        {
            projectName_ = in[EDITOR_KEY_PROJECT_NAME].string();
        }

        // Data sets
        if (in.contains(EDITOR_KEY_DATA_SET))
        {
            datasets_.read(in[EDITOR_KEY_DATA_SET], path_, datasetsFilter_);
        }

        // Layers
        if (in.contains(EDITOR_KEY_LAYER))
        {
            layers_.read(in[EDITOR_KEY_LAYER]);
        }

        // Classifications
        if (in.contains(EDITOR_KEY_CLASSIFICATIONS))
        {
            classifications_.read(in[EDITOR_KEY_CLASSIFICATIONS]);
        }

        // Settings
        if (in.contains(EDITOR_KEY_SETTINGS))
        {
            settings_.read(in[EDITOR_KEY_SETTINGS]);
        }

        // Clip filter
        // if (in.contains(EDITOR_KEY_CLIP_FILTER))
        // {
        //     clipFilter_.read(in[EDITOR_KEY_CLIP_FILTER]);
        // }
        // else
        // {
        //     clipFilter_.clear();
        // }

        // Elevation range
        if (in.contains(EDITOR_KEY_ELEVATION_RANGE))
        {
            elevationFilter_.read(in[EDITOR_KEY_ELEVATION_RANGE]);
        }
    }
    catch (...)
    {
        close();
        throw;
    }

    updateAfterRead();
}

void Editor::saveProject(const std::string &path)
{
    LOG_DEBUG(<< "Save project path <" << path << ">.");

    Json out;

    // Project name
    out[EDITOR_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    datasets_.write(out[EDITOR_KEY_DATA_SET]);

    // Layers
    layers_.write(out[EDITOR_KEY_LAYER]);

    // Classifications
    classifications_.write(out[EDITOR_KEY_CLASSIFICATIONS]);

    // Settings
    settings_.write(out[EDITOR_KEY_SETTINGS]);

    // Clip filter
    // clipFilter_.write(out[EDITOR_KEY_CLIP_FILTER]);

    // Elevation range
    elevationFilter_.write(out[EDITOR_KEY_ELEVATION_RANGE]);

    out.write(path);

    unsavedChanges_ = false;
}

void Editor::openDataset(const std::string &path,
                         const SettingsImport &settings)
{
    LOG_DEBUG(<< "Open dataset path <" << path << ">.");

    try
    {
        datasets_.read(path,
                       path_,
                       settings,
                       datasets_.boundary(),
                       datasetsFilter_);
    }
    catch (...)
    {
        throw;
    }

    updateAfterRead();

    unsavedChanges_ = true;
}

void Editor::setClassifications(const Classifications &classifications)
{
    LOG_DEBUG(<< "Set classifications.");
    classifications_ = classifications;
    // viewClearRendered();
    unsavedChanges_ = true;
}

void Editor::setClassificationsFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set classifications filter.");
    classificationsFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setClassification(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setClipFilter(const Region &clipFilter)
{
    LOG_DEBUG(<< "Set region filter <" << clipFilter << ">.");
    clipFilter_ = clipFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setRegion(clipFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::resetClipFilter()
{
    LOG_DEBUG(<< "Reset region filter.");
    clipFilter_.box = clipFilter_.boundary;
    setClipFilter(clipFilter_);
}

Box<double> Editor::clipBoundary() const
{
    if (clipFilter_.enabled)
    {
        return clipFilter_.box;
    }

    return clipFilter_.boundary;
}

void Editor::setElevationFilter(const Range<double> &elevationFilter)
{
    LOG_DEBUG(<< "Set elevation filter <" << elevationFilter << ">.");
    elevationFilter_ = elevationFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setElevation(elevationFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setDescriptorFilter(const Range<double> &descriptorFilter)
{
    LOG_DEBUG(<< "Set descriptor filter <" << descriptorFilter << ">.");
    descriptorFilter_ = descriptorFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setDescriptor(descriptorFilter_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setDatasets(const Datasets &datasets)
{
    LOG_DEBUG(<< "Set datasets.");
    datasets_ = datasets;
    unsavedChanges_ = true;
}

void Editor::setDatasetsFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set datasets filter.");
    datasetsFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setDataset(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setLayers(const Layers &layers)
{
    LOG_DEBUG(<< "Set layers.");
    layers_ = layers;
    unsavedChanges_ = true;
}

void Editor::setLayersFilter(const QueryFilterSet &filter)
{
    LOG_DEBUG(<< "Set layers filter.");
    layersFilter_ = filter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setLayer(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::updateAfterRead()
{
    LOG_DEBUG(<< "Update after read.");

    clipFilter_.boundary = datasets_.boundary();
    clipFilter_.box = clipFilter_.boundary;
    // clipFilter_.enabled = Region::TYPE_BOX;

    if (viewports_.size() > 0)
    {
        viewports_.where().setDataset(datasetsFilter_);
        viewports_.where().setClassification(classificationsFilter_);
        viewports_.where().setRegion(clipFilter_);
        viewports_.where().setElevation(elevationFilter_);
        viewports_.where().setDescriptor(descriptorFilter_);
        viewports_.where().setLayer(layersFilter_);

        viewports_.applyWhereToAll();
    }
}

void Editor::setSettingsView(const SettingsView &settings)
{
    settings_.setView(settings);
    unsavedChanges_ = true;
}

void Editor::addModifier(ModifierInterface *modifier)
{
    modifiers_.push_back(modifier);
}

void Editor::runModifiers(Page *page)
{
    for (auto &it : modifiers_)
    {
        /** @todo Collect enabled modifiers during preprocessing. */
        if (it->isModifierEnabled())
        {
            it->applyModifier(page);
        }
    }
}

void Editor::viewportsResize(size_t n)
{
    LOG_DEBUG(<< "Set number of viewports to <" << n << ">.");
    viewports_.resize(this, n);
    viewports_.applyWhereToAll();
}

std::shared_ptr<PageData> Editor::readPage(size_t dataset, size_t index)
{
    return pageManager_.get(this, dataset, index);
}

void Editor::erasePage(size_t dataset, size_t index)
{
    pageManager_.erase(this, dataset, index);
}

void Editor::lock()
{
    mutex_.lock();
}

void Editor::unlock()
{
    mutex_.unlock();
}
