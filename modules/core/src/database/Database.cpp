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
    @file Database.cpp
*/

#include <Database.hpp>
#include <iostream>
#include <limits>
#include <queue>

Database::Database() : cacheSizeMax_(100)
{
}

Database::~Database()
{
}

void Database::addDataSet(size_t id, const std::string &path, bool enabled)
{
    std::shared_ptr<DatabaseDataSet> dataSet;
    dataSet = std::make_shared<DatabaseDataSet>();
    dataSet->read(id, path, enabled);
    dataSets_[id] = dataSet;
    updateBoundary();
}

void Database::clear()
{
    dataSets_.clear();
    boundary_.clear();
    boundaryView_.clear();

    cache_.clear();

    view_.clear();
}

void Database::select(std::vector<OctreeIndex::Selection> &selection)
{
    for (auto const &it : dataSets_)
    {
        if (it.second->enabled_)
        {
            // TBD: data set id
            it.second->index_.selectNodes(selection, boundary_);
        }
    }
}

void Database::updateCamera(const Camera &camera, bool interactionFinished)
{
    if (!interactionFinished)
    {
        resetRendering();
        return;
    }

    double eyeX = camera.eye.x();
    double eyeY = camera.eye.y();
    double eyeZ = camera.eye.z();

    std::vector<std::shared_ptr<DatabaseCell>> viewPrev;
    viewPrev = view_;

    view_.clear();

    std::multimap<double, Key> queue;

    for (auto const &it : dataSets_)
    {
        if (it.second->enabled_)
        {
            queue.insert({0, {it.first, 0}});
        }
    }

    while (!queue.empty() && view_.size() < cacheSizeMax_)
    {
        const auto it = queue.begin();
        Key nk = it->second;
        queue.erase(it);

        const OctreeIndex::Node *node;
        const OctreeIndex &index = dataSets_[nk.dataSetId]->index_;
        node = index.at(nk.cellId);

        auto search = cache_.find({nk.dataSetId, nk.cellId});
        if (search != cache_.end())
        {
            view_.push_back(search->second);
        }
        else
        {
            if (viewPrev.size() > 0)
            {
                const DatabaseCell *lru = viewPrev[viewPrev.size() - 1].get();
                Key nkrm = {lru->dataSetId, lru->cellId};
                cache_.erase(nkrm);
                viewPrev.resize(viewPrev.size() - 1);
            }

            std::shared_ptr<DatabaseCell> cell;
            cell = std::make_shared<DatabaseCell>();
            cell->dataSetId = nk.dataSetId;
            cell->cellId = nk.cellId;
            cell->loaded = false;
            cache_[nk] = cell;
            view_.push_back(cell);
        }

        for (size_t i = 0; i < 8; i++)
        {
            if (node->next[i])
            {
                const OctreeIndex::Node *sub = index.at(node->next[i]);
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

bool Database::loadView()
{
    for (size_t i = 0; i < view_.size(); i++)
    {
        if (!view_[i]->loaded)
        {
            loadView(i);
            return false;
        }

        if (!view_[i]->view.isFinished())
        {
            return false;
        }
    }

    return true;
}

bool Database::isCached(size_t d, size_t c) const
{
    auto search = cache_.find({d, c});
    return search != cache_.end();
}

DatabaseCell *Database::get(size_t d, size_t c) const
{
    auto search = cache_.find({d, c});
    if (search != cache_.end())
    {
        return search->second.get();
    }
    return nullptr;
}

void Database::updateBoundary()
{
    boundary_.clear();
    boundaryView_.clear();

    for (auto const &it : dataSets_)
    {
        if (it.second->enabled_)
        {
            boundary_.extend(it.second->boundary_);
            boundaryView_.extend(it.second->boundaryView_);
        }
    }
}

bool Database::Key::operator<(const Key &rhs) const
{
    return (dataSetId < rhs.dataSetId) || (cellId < rhs.cellId);
}

void Database::loadView(size_t idx)
{
    DatabaseCell *cell = view_[idx].get();
    DatabaseDataSet *dataSet = dataSets_[cell->dataSetId].get();
    const OctreeIndex::Node *node = dataSet->index_.at(cell->cellId);

    LasFile las;
    las.open(dataSet->path_);
    las.readHeader();

    size_t n = static_cast<size_t>(node->size);
    bool rgbFlag = las.header.hasRgb();
    std::vector<double> &xyz = cell->xyz;
    std::vector<float> &xyzView = cell->view.xyz;
    std::vector<float> &rgb = cell->rgb;
    xyz.resize(n * 3);
    xyzView.resize(n * 3);
    if (rgbFlag)
    {
        rgb.resize(n * 3);
    }

    size_t pointSize = las.header.point_data_record_length;
    uint64_t start = node->from * pointSize;
    las.seek(start + las.header.offset_to_point_data);

    std::vector<uint8_t> buffer;
    size_t bufferSize = pointSize * n;
    uint8_t fmt = las.header.point_data_record_format;
    buffer.resize(bufferSize);
    las.file().read(buffer.data(), bufferSize);

    uint8_t *ptr = buffer.data();
    LasFile::Point point;
    double x;
    double y;
    double z;
    constexpr float scaleU16 =
        1.0F / static_cast<float>(std::numeric_limits<uint16_t>::max());

    for (size_t i = 0; i < n; i++)
    {
        las.readPoint(point, ptr + (pointSize * i), fmt);

        las.transform(x, y, z, point);

        xyz[3 * i + 0] = x;
        xyz[3 * i + 1] = y;
        xyz[3 * i + 2] = z;

        xyzView[3 * i + 0] = static_cast<float>(static_cast<double>(point.x) +
                                                las.header.x_offset);
        xyzView[3 * i + 1] = static_cast<float>(static_cast<double>(point.y) +
                                                las.header.y_offset);
        xyzView[3 * i + 2] = static_cast<float>(static_cast<double>(point.z) +
                                                las.header.z_offset);

        if (rgbFlag)
        {
            rgb[3 * i + 0] = point.red * scaleU16;
            rgb[3 * i + 1] = point.green * scaleU16;
            rgb[3 * i + 2] = point.blue * scaleU16;
        }
    }

    cell->view.rgb = rgb;

    cell->boundary.set(xyz);
    cell->view.boundary.set(xyzView);

    cell->loaded = true;
}

void Database::resetRendering()
{
    for (size_t i = 0; i < view_.size(); i++)
    {
        view_[i]->view.resetFrame();
    }
}
