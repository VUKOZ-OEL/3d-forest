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

/** @file EditorBase.cpp */

#include <EditorBase.hpp>
#include <FileIndexBuilder.hpp>

static const char *EDITOR_BASE_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_BASE_KEY_DATA_SET = "dataSets";
static const char *EDITOR_BASE_KEY_LAYER = "layers";
static const char *EDITOR_BASE_KEY_SETTINGS = "settings";
static const char *EDITOR_BASE_KEY_CLASSIFICATIONS = "classifications";
// static const char *EDITOR_BASE_KEY_CLIP_FILTER = "clipFilter";

EditorBase::EditorBase() : working_(this)
{
    close();
    setNumberOfViewports(1);
}

EditorBase::~EditorBase()
{
}

void EditorBase::close()
{
    path_ = File::currentPath() + "\\untitled.json";
    projectName_ = "Untitled";

    database_.clear();
    dataSets_.clear();
    layers_.clear();
    classifications_.clear();
    clipFilter_.clear();

    for (auto &it : viewports_)
    {
        it->clear();
    }
    working_.clear();

    unsavedChanges_ = false;
}

void EditorBase::open(const std::string &path)
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
        if (in.contains(EDITOR_BASE_KEY_PROJECT_NAME))
        {
            projectName_ = in[EDITOR_BASE_KEY_PROJECT_NAME].string();
        }

        // Data sets
        if (in.contains(EDITOR_BASE_KEY_DATA_SET))
        {
            dataSets_.read(in[EDITOR_BASE_KEY_DATA_SET], path_);
            database_.setDataSets(dataSets_);
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
        close();
        throw;
    }

    openUpdate();
}

void EditorBase::write(const std::string &path)
{
    Json out;

    // Project name
    out[EDITOR_BASE_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    dataSets_.write(out[EDITOR_BASE_KEY_DATA_SET]);

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

void EditorBase::addFile(const std::string &path,
                         const EditorSettingsImport &settings)
{
    try
    {
        // Data sets
        size_t id = dataSets_.unusedId();
        dataSets_.read(id, path, path_, settings, database_.boundary());

        // Database
        std::shared_ptr<EditorDatabase> db = std::make_shared<EditorDatabase>();
        db->setProperties(dataSets_.at(dataSets_.size() - 1));
        database_.push_back(db);
    }
    catch (std::exception &e)
    {
        throw;
    }

    openUpdate();
    unsavedChanges_ = true;
}

bool EditorBase::hasFileIndex(const std::string &path)
{
    std::string pathFile = File::resolvePath(path, path_);
    std::string pathIndex = FileIndexBuilder::extension(pathFile);
    return File::exists(pathIndex);
}

void EditorBase::addFilter(EditorFilter *filter)
{
    filters_.push_back(filter);
}

void EditorBase::applyFilters(EditorTile *tile)
{
    for (auto &it : filters_)
    {
        /** @todo Collect enabled filters during preprocessing. */
        if (it->isFilterEnabled())
        {
            it->filterTile(tile);
        }
    }
}

void EditorBase::setDataSets(const EditorDataSets &dataSets)
{
    dataSets_ = dataSets;
    database_.setDataSets(dataSets);
    unsavedChanges_ = true;
}

void EditorBase::setLayers(const EditorLayers &layers)
{
    layers_ = layers;
    unsavedChanges_ = true;
}

void EditorBase::setClassifications(
    const EditorClassifications &classifications)
{
    classifications_ = classifications;
    // resetRendering();
    unsavedChanges_ = true;
}

void EditorBase::setClipFilter(const ClipFilter &clipFilter)
{
    clipFilter_ = clipFilter;
    clipFilter_.boxView.setPercent(database_.boundaryView(),
                                   database_.boundary(),
                                   clipFilter_.box);

    // unsavedChanges_ = true;
}

void EditorBase::resetClipFilter()
{
    clipFilter_.box = database_.boundary();
    setClipFilter(clipFilter_);
}

void EditorBase::setSettingsView(const EditorSettingsView &settings)
{
    settings_.setView(settings);
    resetRendering();
    unsavedChanges_ = true;
}

void EditorBase::select(std::vector<FileIndex::Selection> &selected)
{
    database_.select(selected, selection());
}

Aabb<double> EditorBase::selection() const
{
    if (clipFilter_.enabled)
    {
        return clipFilter_.box;
    }

    return boundary();
}

void EditorBase::setNumberOfViewports(size_t n)
{
    size_t i = viewports_.size();

    while (i < n)
    {
        std::shared_ptr<EditorCache> viewport =
            std::make_shared<EditorCache>(this);
        viewports_.push_back(viewport);
        i++;
    }

    while (n < i)
    {
        viewports_.pop_back();
        i--;
    }
}

void EditorBase::updateCamera(size_t viewport, const Camera &camera)
{
    viewports_[viewport]->updateCamera(camera);
}

void EditorBase::tileViewClear()
{
    for (auto &it : viewports_)
    {
        it->reload();
    }
}

bool EditorBase::loadView()
{
    bool rval = true;

    for (auto &it : viewports_)
    {
        if (!it->loadStep())
        {
            rval = false;
        }
    }

    return rval;
}

void EditorBase::openUpdate()
{
    // if (clipFilter_.box.empty())
    // {
    //     clipFilter_.box = boundary_;
    // }
    clipFilter_.box = boundary();
    clipFilter_.boxView = boundaryView();
}

void EditorBase::resetRendering()
{
    for (auto &it : viewports_)
    {
        it->resetRendering();
    }
}
