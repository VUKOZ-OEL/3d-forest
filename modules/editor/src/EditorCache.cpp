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
    @file EditorCache.cpp
*/

#include <EditorBase.hpp>
#include <EditorCache.hpp>
#include <Error.hpp>
#include <queue>

EditorCache::EditorCache(EditorBase *editor) : editor_(editor)
{
    cacheSizeMax_ = 250;
}

EditorCache::~EditorCache()
{
}

void EditorCache::clear()
{
    cache_.clear();
    lru_.clear();
}

void EditorCache::reload()
{
    for (auto &it : cache_)
    {
        it.second->view.resetFrame();
        it.second->filtered = false;
        // it.second->loaded = false;
    }
}

bool EditorCache::loadStep()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        if (!lru_[i]->loaded)
        {
            load(i);
            editor_->applyFilters(lru_[i].get());
            return false;
        }

        if (!lru_[i]->filtered)
        {
            lru_[i]->filter(editor_);
            editor_->applyFilters(lru_[i].get());
            return false;
        }

        if (!lru_[i]->view.isFinished())
        {
            return false;
        }
    }

    return true;
}

void EditorCache::load(size_t idx)
{
    EditorTile *tile = lru_[idx].get();
    try
    {
        tile->read(editor_);
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

void EditorCache::updateCamera(const Camera &camera)
{
    double eyeX = camera.eye.x();
    double eyeY = camera.eye.y();
    double eyeZ = camera.eye.z();

    std::vector<std::shared_ptr<EditorTile>> viewPrev;
    viewPrev = lru_;

    lru_.clear();

    std::multimap<double, Key> queue;

    for (size_t i = 0; i < editor_->dataSetSize(); i++)
    {
        const EditorDataSet &ds = editor_->dataSet(i);
        if (ds.visible)
        {
            queue.insert({0, {ds.id, 0}});
        }
    }

    while (!queue.empty() && lru_.size() < cacheSizeMax_)
    {
        const auto it = queue.begin();
        Key nk = it->second;
        queue.erase(it);

        const EditorDataSet &ds = editor_->dataSet(nk.dataSetId);
        const FileIndex::Node *node;
        const FileIndex &index = ds.index;
        node = index.at(nk.tileId);

        if (editor_->clipFilter().enabled)
        {
            Aabb<double> box = index.boundary(node, index.boundary());
            if (!editor_->clipFilter().box.intersects(box))
            {
                continue;
            }
        }

        auto search = cache_.find({nk.dataSetId, nk.tileId});
        if (search != cache_.end())
        {
            lru_.push_back(search->second);
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
            lru_.push_back(tile);
        }

        for (size_t i = 0; i < 8; i++)
        {
            if (node->next[i])
            {
                const FileIndex::Node *sub = index.at(node->next[i]);
                Aabb<double> box = index.boundary(sub, editor_->boundaryView());

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

void EditorCache::resetRendering()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        lru_[i]->view.resetFrame();
    }
}

bool EditorCache::Key::operator<(const Key &rhs) const
{
    return (dataSetId < rhs.dataSetId) || (tileId < rhs.tileId);
}

#if 0
bool isCached(size_t d, size_t c) const;
bool EditorCache::isCached(size_t d, size_t c) const
{
    auto search = cache_.find({d, c});
    return search != cache_.end();
}
#endif
