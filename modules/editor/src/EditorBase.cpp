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

/**
    @file EditorBase.cpp
*/

#include <EditorBase.hpp>
#include <Error.hpp>
#include <iostream>
#include <limits>
#include <queue>

static const char *EDITOR_BASE_KEY_PROJECT_NAME = "projectName";
static const char *EDITOR_BASE_KEY_DATA_SET = "dataSets";
static const char *EDITOR_BASE_KEY_LAYER = "layers";
// static const char *EDITOR_BASE_KEY_CLIP_FILTER = "clipFilter";

EditorBase::EditorBase()
{
    unsavedChanges_ = false;
    cacheSizeMax_ = 250;
}

EditorBase::~EditorBase()
{
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
        size_t i;
        size_t n;

        path_ = path;

        // Project name
        if (in.contains(EDITOR_BASE_KEY_PROJECT_NAME))
        {
            projectName_ = in[EDITOR_BASE_KEY_PROJECT_NAME].string();
        }
        else
        {
            projectName_ = "Untitled";
        }

        // Data sets
        if (in.contains(EDITOR_BASE_KEY_DATA_SET))
        {
            i = 0;
            n = in[EDITOR_BASE_KEY_DATA_SET].array().size();
            dataSets_.resize(n);

            for (auto const &it : in[EDITOR_BASE_KEY_DATA_SET].array())
            {
                dataSets_[i] = std::make_shared<EditorDataSet>();
                dataSets_[i]->read(it, path_);
                i++;
            }
        }

        // Layers
        if (in.contains(EDITOR_BASE_KEY_LAYER))
        {
            i = 0;
            n = in[EDITOR_BASE_KEY_LAYER].array().size();
            layers_.resize(n);

            for (auto const &it : in[EDITOR_BASE_KEY_LAYER].array())
            {
                layers_[i].read(it);
                i++;
            }
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

void EditorBase::openFile(const std::string &path)
{
    close();

    try
    {
        // Data sets
        dataSets_.resize(1);
        dataSets_[0] = std::make_shared<EditorDataSet>();
        dataSets_[0]->read(path);
    }
    catch (std::exception &e)
    {
        close();
        throw;
    }

    openUpdate();
}

void EditorBase::openUpdate()
{
    updateBoundary();

    if (clipFilter_.box.empty())
    {
        clipFilter_.box = boundary_;
    }
}

void EditorBase::write(const std::string &path)
{
    Json out;
    size_t i;

    // Project name
    out[EDITOR_BASE_KEY_PROJECT_NAME] = projectName_;

    // Data sets
    i = 0;
    for (auto const &it : dataSets_)
    {
        it->write(out[EDITOR_BASE_KEY_DATA_SET][i]);
        i++;
    }

    // Layers
    i = 0;
    for (auto const &it : layers_)
    {
        it.write(out[EDITOR_BASE_KEY_LAYER][i]);
        i++;
    }

    // Clip filter
    // clipFilter_.write(out[EDITOR_BASE_KEY_CLIP_FILTER]);

    out.write(path);

    unsavedChanges_ = false;
}

void EditorBase::close()
{
    path_ = "";
    projectName_ = "";
    dataSets_.clear();
    layers_.clear();
    clipFilter_.clear();

    dataSets_.clear();
    boundary_.clear();
    boundaryView_.clear();
    cache_.clear();
    view_.clear();

    unsavedChanges_ = false;
}

void EditorBase::addFilter(EditorFilter *filter)
{
    filters_.push_back(filter);
}

void EditorBase::applyFilters(EditorTile *tile)
{
    for (auto &it : filters_)
    {
        // TBD: collect enabled filters during preprocessing
        if (it->isFilterEnabled())
        {
            it->filterTile(tile);
        }
    }
}

void EditorBase::setSettingsView(const EditorSettings::View &settings)
{
    settings_.setView(settings);
    // unsavedChanges_ = true;
}

void EditorBase::setVisibleDataSet(size_t i, bool visible)
{
    dataSets_[i]->visible = visible;
    unsavedChanges_ = true;
}

void EditorBase::setVisibleLayer(size_t i, bool visible)
{
    layers_[i].visible = visible;
    unsavedChanges_ = true;
}

void EditorBase::setClipFilter(const ClipFilter &clipFilter)
{
    clipFilter_ = clipFilter;
    clipFilter_.boxView.setPercent(boundaryView_, boundary_, clipFilter_.box);

    // unsavedChanges_ = true;
}

void EditorBase::resetClipFilter()
{
    clipFilter_.box = boundary_;
    setClipFilter(clipFilter_);
}

void EditorBase::select(std::vector<FileIndex::Selection> &selected)
{
    Aabb<double> box = selection();

    for (auto const &it : dataSets_)
    {
        if (it->visible)
        {
            it->index.selectNodes(selected, box, it->id);
        }
    }
}

Aabb<double> EditorBase::selection() const
{
    if (clipFilter_.enabled)
    {
        return clipFilter_.box;
    }

    return boundary_;
}

bool EditorBase::isCached(size_t d, size_t c) const
{
    auto search = cache_.find({d, c});
    return search != cache_.end();
}

EditorTile *EditorBase::tile(size_t d, size_t c)
{
    auto search = cache_.find({d, c});
    if (search != cache_.end())
    {
        return search->second.get();
    }
    return nullptr;
}

void EditorBase::updateBoundary()
{
    boundary_.clear();
    boundaryView_.clear();

    for (auto const &it : dataSets_)
    {
        if (it->visible)
        {
            boundary_.extend(it->boundary);
            boundaryView_.extend(it->boundaryView);
        }
    }
}

bool EditorBase::Key::operator<(const Key &rhs) const
{
    return (dataSetId < rhs.dataSetId) || (tileId < rhs.tileId);
}

void EditorBase::tileViewClear()
{
    for (auto &it : cache_)
    {
        it.second->loaded = false;
    }
}

bool EditorBase::loadView()
{
    for (size_t i = 0; i < view_.size(); i++)
    {
        if (!view_[i]->loaded)
        {
            loadView(i);
            applyFilters(view_[i].get());
            return false;
        }

        if (!view_[i]->view.isFinished())
        {
            return false;
        }
    }

    return true;
}

void EditorBase::loadView(size_t idx)
{
    EditorTile *tile = view_[idx].get();
    try
    {
        tile->read(this);
    }
    catch (std::exception &e)
    {
        // std::cout << e.what() << "\n";
    }
    catch (...)
    {
        // std::cout << "unknown error\n";
    }
}

void EditorBase::updateCamera(const Camera &camera)
{
    double eyeX = camera.eye.x();
    double eyeY = camera.eye.y();
    double eyeZ = camera.eye.z();

    std::vector<std::shared_ptr<EditorTile>> viewPrev;
    viewPrev = view_;

    view_.clear();

    std::multimap<double, Key> queue;

    for (auto const &it : dataSets_)
    {
        if (it->visible)
        {
            queue.insert({0, {it->id, 0}});
        }
    }

    while (!queue.empty() && view_.size() < cacheSizeMax_)
    {
        const auto it = queue.begin();
        Key nk = it->second;
        queue.erase(it);

        const FileIndex::Node *node;
        const FileIndex &index = dataSets_[nk.dataSetId]->index;
        node = index.at(nk.tileId);

        // if (clipFilter_.enabled)
        // {
        //     Aabb<double> box = index.boundary(node, index.boundary());
        //     if (!clipFilter_.box.intersects(box))
        //     {
        //         continue;
        //     }
        // }

        auto search = cache_.find({nk.dataSetId, nk.tileId});
        if (search != cache_.end())
        {
            view_.push_back(search->second);
        }
        else
        {
            if (viewPrev.size() > 0)
            {
                const EditorTile *lru = viewPrev[viewPrev.size() - 1].get();
                Key nkrm = {lru->dataSetId, lru->tileId};
                cache_.erase(nkrm);
                viewPrev.resize(viewPrev.size() - 1);
            }

            std::shared_ptr<EditorTile> tile;
            tile = std::make_shared<EditorTile>();
            tile->dataSetId = nk.dataSetId;
            tile->tileId = nk.tileId;
            tile->loaded = false;
            cache_[nk] = tile;
            view_.push_back(tile);
        }

        for (size_t i = 0; i < 8; i++)
        {
            if (node->next[i])
            {
                const FileIndex::Node *sub = index.at(node->next[i]);
                Aabb<double> box = index.boundary(sub, boundaryView_);

                double radius = box.radius();
                double distance = box.distance(eyeX, eyeY, eyeZ);
                double w;

                if (distance < radius)
                {
                    w = 0;
                }
                else
                {
                    distance = distance * 0.002;
                    distance = distance * distance;
                    w = distance / radius;
                }

                queue.insert({w, {nk.dataSetId, node->next[i]}});
            }
        }
    }

    resetRendering();
}

void EditorBase::resetRendering()
{
    for (size_t i = 0; i < view_.size(); i++)
    {
        view_[i]->view.resetFrame();
    }
}
