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
    close();
    viewportsResize(1);
}

Editor::~Editor()
{
}

void Editor::close()
{
    path_ = File::join(File::currentPath(), "untitled.json");
    projectName_ = "Untitled";

    datasets_.clear();
    layers_.setDefault();
    classifications_.clear();
    voxels_.clear();
    viewports_.clearContent();

    clipFilter_.clear();
    elevationRange_.clear();
    densityRange_.set(0.0, 1.0, 0.0, 1.0);
    descriptorRange_.set(0.0, 1.0, 0.0, 1.0);

    unsavedChanges_ = false;
}

void Editor::open(const std::string &path, const SettingsImport &settings)
{
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
            datasets_.read(in[EDITOR_KEY_DATA_SET], path_);
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
            elevationRange_.read(in[EDITOR_KEY_ELEVATION_RANGE]);
        }
    }
    catch (std::exception &e)
    {
        close();
        throw;
    }

    updateAfterRead();
}

void Editor::save(const std::string &path)
{
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
    elevationRange_.write(out[EDITOR_KEY_ELEVATION_RANGE]);

    out.write(path);

    unsavedChanges_ = false;
}

void Editor::openDataset(const std::string &path,
                         const SettingsImport &settings)
{
    try
    {
        LOG_DEBUG(<< "Called with parameter path <" << path << ">.");
        datasets_.read(path, path_, settings, datasets_.boundary());
    }
    catch (std::exception &e)
    {
        throw;
    }

    updateAfterRead();

    unsavedChanges_ = true;
}

void Editor::setClassifications(const Classifications &classifications)
{
    classifications_ = classifications;
    // viewClearRendered();
    unsavedChanges_ = true;
}

void Editor::setClassificationsFilter(const QueryFilterSet &filter)
{
    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setClassification(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setClipFilter(const Region &clipFilter)
{
    LOG_DEBUG(<< "Set region <" << clipFilter << ">.");
    clipFilter_ = clipFilter;

    if (viewports_.size() > 0)
    {
        viewports_.where().setBox(clipBoundary());
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::resetClipFilter()
{
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

void Editor::setElevationRange(const Range<double> &elevationRange)
{
    elevationRange_ = elevationRange;
    LOG_DEBUG(<< "Called with parameter elevationRange <" << elevationRange_
              << ">.");

    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setElevation(elevationRange_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::setDensityRange(const Range<double> &densityRange)
{
    densityRange_ = densityRange;
    LOG_DEBUG(<< "Called with parameter densityRange <" << densityRange_
              << ">.");

    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setDensity(densityRange_);
        viewports_.applyWhereToAll();
    }
}

void Editor::setDescriptorRange(const Range<double> &descriptorRange)
{
    descriptorRange_ = descriptorRange;
    LOG_DEBUG(<< "Called with parameter descriptorRange <" << descriptorRange_
              << ">.");

    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setDescriptor(descriptorRange_);
        viewports_.applyWhereToAll();
    }

    // unsavedChanges_ = true;
}

void Editor::updateAfterRead()
{
    clipFilter_.boundary = datasets_.boundary();
    clipFilter_.box = clipFilter_.boundary;

    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        // viewports_.where().setDataset();
        // viewports_.where().setClassification();
        viewports_.where().setBox(clipBoundary());
        viewports_.where().setElevation(elevationRange_);
        viewports_.where().setDensity(densityRange_);
        viewports_.where().setDescriptor(descriptorRange_);
        // viewports_.where().setLayer();

        viewports_.applyWhereToAll();
    }
}

void Editor::setDatasets(const Datasets &datasets)
{
    LOG_DEBUG(<< "Called.");
    datasets_ = datasets;
    unsavedChanges_ = true;
}

void Editor::setDatasetsFilter(const QueryFilterSet &filter)
{
    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setDataset(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setLayers(const Layers &layers)
{
    layers_ = layers;
    unsavedChanges_ = true;
}

void Editor::setLayersFilter(const QueryFilterSet &filter)
{
    if (viewports_.size() > 0)
    {
        LOG_DEBUG(<< "Called.");
        viewports_.where().setLayer(filter);
        viewports_.applyWhereToAll();
    }
}

void Editor::setVoxels(const Voxels &voxels)
{
    voxels_ = voxels;
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
    viewports_.resize(this, n);
}

void Editor::lock()
{
    mutex_.lock();
}

void Editor::unlock()
{
    mutex_.unlock();
}
