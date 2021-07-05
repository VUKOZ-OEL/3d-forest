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

/** @file EditorCache.cpp */

#include <EditorBase.hpp>
#include <EditorCache.hpp>
#include <Error.hpp>
#include <queue>

EditorCache::EditorCache(EditorBase *editor) : editor_(editor)
{
    cacheSizeMax_ = 200;
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

        if (!lru_[i]->transformed)
        {
            lru_[i]->transform(editor_);
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
    double eyeX = camera.eye[0];
    double eyeY = camera.eye[1];
    double eyeZ = camera.eye[2];

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
            box.translate(ds.translation);
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

EditorTile *EditorCache::tile(size_t dataset, size_t index)
{
    Key nk = {dataset, index};

    auto search = cache_.find(nk);
    if (search != cache_.end())
    {
        // Move found tile to top
        for (size_t i = 0; i < lru_.size(); i++)
        {
            if (lru_[i] == search->second)
            {
                if (i > 0)
                {
                    std::shared_ptr<EditorTile> tmp = lru_[i];
                    for (size_t j = i; j > 0; j--)
                    {
                        lru_[j] = lru_[j - 1];
                    }
                    lru_[0] = tmp;
                }

                break;
            }
        }

        return search->second.get();
    }
    else
    {
        // LRU
        if (lru_.size() > 0)
        {
            size_t idx;

            if (lru_.size() < cacheSizeMax_)
            {
                // Make room for new tile
                lru_.resize(lru_.size() + 1);
                idx = lru_.size() - 1;
            }
            else
            {
                // Drop oldest tile
                idx = lru_.size() - 1;
                Key nkrm = {lru_[idx]->dataSetId, lru_[idx]->tileId};
                cache_.erase(nkrm);
            }

            // New tile is on top
            for (size_t j = idx; j > 0; j--)
            {
                lru_[j] = lru_[j - 1];
            }
        }
        else
        {
            // First tile is on top
            lru_.resize(1);
        }

        std::shared_ptr<EditorTile> tile = std::make_shared<EditorTile>();
        tile->dataSetId = nk.dataSetId;
        tile->tileId = nk.tileId;
        tile->loaded = false;
        cache_[nk] = tile;
        lru_[0] = tile;

        try
        {
            tile->read(editor_);
        }
        catch (...)
        {
            // error
        }

        return tile.get();
    }
}
