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

/** @file EditorQuery.cpp */

#include <EditorDatabase.hpp>
#include <EditorQuery.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <queue>

EditorQuery::EditorQuery(EditorDatabase *editor) : editor_(editor)
{
    cacheSizeMax_ = 200;
}

EditorQuery::~EditorQuery()
{
}

void EditorQuery::exec()
{
    selectedPages_.clear();
    editor_->datasets().select(selectedPages_, selectBox_);
    reset();
    setStateSelect();
}

void EditorQuery::reset()
{
    pageIndex_ = 0;
    pagePointIndex_ = 0;
    pagePointIndexMax_ = 0;
}

void EditorQuery::clear()
{
    cache_.clear();
    lru_.clear();

    page_.reset();

    selectedPages_.clear();

    reset();
}

bool EditorQuery::nextPage()
{
    LOG_EDITOR_QUERY("");

    // Reset point index within active page.
    pagePointIndex_ = 0;
    pagePointIndexMax_ = 0;

    // Find next page in selection.
    while (pageIndex_ < selectedPages_.size())
    {
        FileIndex::Selection &selectedPage = selectedPages_[pageIndex_];
        page_ = read(selectedPage.id, selectedPage.idx);
        page_->nextState();
        pageIndex_++;

        if (page_.get() && page_->selection.size() > 0)
        {
            // This page is in selection.

            // Set point index range within the page.
            pagePointIndexMax_ = page_->selection.size() - 1;

            // Point to current page data.
            position_ = page_->position.data();
            intensity_ = page_->intensity.data();
            returnNumber_ = page_->returnNumber.data();
            numberOfReturns_ = page_->numberOfReturns.data();
            classification_ = page_->classification.data();
            userData_ = page_->userData.data();
            gpsTime_ = page_->gpsTime.data();
            color_ = page_->color.data();
            userColor_ = page_->userColor.data();
            layer_ = page_->layer.data();

            selection_ = page_->selection.data();

            return true;
        }
    }

    // There are no more pages in selection.
    return false;
}

size_t EditorQuery::pageSizeEstimate() const
{
    return selectedPages_.size();
}

void EditorQuery::setModified()
{
    page_->setModified();
}

void EditorQuery::write()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        if (lru_[i]->isModified())
        {
            lru_[i]->write();
        }
    }
}

void EditorQuery::setStateRead()
{
    for (auto &it : cache_)
    {
        it.second->setStateRead();
    }
}

void EditorQuery::setStateSelect()
{
    for (auto &it : cache_)
    {
        it.second->setStateSelect();
    }
}

void EditorQuery::setStateRender()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        lru_[i]->setStateRender();
    }
}

bool EditorQuery::nextState()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        bool finished = lru_[i]->nextState();
        if (!finished)
        {
            return false;
        }
    }

    return true;
}

void EditorQuery::selectBox(const Box<double> &box)
{
    selectBox_ = box;
}

void EditorQuery::selectCamera(const Camera &camera)
{
    double eyeX = camera.eye[0];
    double eyeY = camera.eye[1];
    double eyeZ = camera.eye[2];

    std::vector<std::shared_ptr<EditorPage>> viewPrev;
    viewPrev = lru_;

    lru_.clear();

    std::multimap<double, Key> queue;

    for (size_t i = 0; i < editor_->datasets().size(); i++)
    {
        const EditorDataset &db = editor_->datasets().at(i);
        if (db.isEnabled())
        {
            queue.insert({0, {db.id(), 0}});
        }
    }

    while (!queue.empty() && lru_.size() < cacheSizeMax_)
    {
        const auto it = queue.begin();
        Key nk = it->second;
        queue.erase(it);

        const EditorDataset &db = editor_->datasets().key(nk.datasetId);
        const FileIndex::Node *node;
        const FileIndex &index = db.index();
        node = index.at(nk.pageId);

        if (editor_->clipFilter().enabled)
        {
            Box<double> box = index.boundary(node, index.boundary());
            if (!editor_->clipFilter().box.intersects(box))
            {
                continue;
            }
        }

        auto search = cache_.find({nk.datasetId, nk.pageId});
        if (search != cache_.end())
        {
            lru_.push_back(search->second);
        }
        else
        {
            if (viewPrev.size() > 0)
            {
                const EditorPage *lru = viewPrev[viewPrev.size() - 1].get();
                Key nkrm = {lru->datasetId(), lru->pageId()};
                cache_.erase(nkrm);
                viewPrev.resize(viewPrev.size() - 1);
            }

            std::shared_ptr<EditorPage> page;
            page = std::make_shared<EditorPage>(editor_,
                                                this,
                                                nk.datasetId,
                                                nk.pageId);
            cache_[nk] = page;
            lru_.push_back(page);
        }

        for (size_t i = 0; i < 8; i++)
        {
            if (node->next[i])
            {
                const FileIndex::Node *sub = index.at(node->next[i]);
                Box<double> box = index.boundary(sub, editor_->clipBoundary());

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

                queue.insert({w, {nk.datasetId, node->next[i]}});
            }
        }
    }

    setStateRender();
}

static void editorQueryCreateGrid(std::vector<uint64_t> &grid,
                                  size_t x1,
                                  size_t x2,
                                  size_t y1,
                                  size_t y2)
{
    size_t dx = x2 - x1;
    size_t dy = y2 - y1;

    if (dx < 1 || dy < 1)
    {
        return;
    }

    if (dx == 1 && dy == 1)
    {
        // 0xfffff = 20 bits = 1,048,575 cells per length.
        uint64_t value = (x1 & 0xfffffU) | ((y1 & 0xfffffU) << 20);

        grid.push_back(value);

        return;
    }

    size_t px = dx / 2;
    size_t py = dy / 2;

    editorQueryCreateGrid(grid, x1, x1 + px, y1, y1 + py);
    editorQueryCreateGrid(grid, x1 + px, x2, y1, y1 + py);
    editorQueryCreateGrid(grid, x1, x1 + px, y1 + py, y2);
    editorQueryCreateGrid(grid, x1 + px, x2, y1 + py, y2);
}

void EditorQuery::setGrid(size_t pointsPerCell)
{
    LOG_EDITOR_QUERY("");

    // Calculate grid cell size.
    uint64_t pointsPerArea = editor_->datasets().nPoints();
    Box<double> boundary = editor_->boundary();
    double area = boundary.length(0) * boundary.length(1);

    Box<double> boundaryClip = editor_->clipBoundary();
    double areaClip = boundaryClip.length(0) * boundaryClip.length(1);
    double areaRatio = areaClip / area;
    double pointsPerAreaClip = static_cast<double>(pointsPerArea) * areaRatio;
    double nCells = pointsPerAreaClip / static_cast<double>(pointsPerCell);
    nCells = ceil(nCells);
    LOG_EDITOR_QUERY("nCells:" << nCells);

    double areaPerCell = areaClip / nCells;
    double cellLength = sqrt(areaPerCell);

    gridXSize_ =
        static_cast<size_t>(round(boundaryClip.length(0) / cellLength));
    LOG_EDITOR_QUERY("gridXSize_:" << gridXSize_);
    gridYSize_ =
        static_cast<size_t>(round(boundaryClip.length(1) / cellLength));
    LOG_EDITOR_QUERY("gridYSize_:" << gridYSize_);

    double cellLengthX =
        boundaryClip.length(0) / static_cast<double>(gridXSize_);
    LOG_EDITOR_QUERY("cellLengthX:" << cellLengthX);
    double cellLengthY =
        boundaryClip.length(1) / static_cast<double>(gridYSize_);
    LOG_EDITOR_QUERY("cellLengthY:" << cellLengthY);

    // Set grid cell size.
    gridBoundary_ = boundaryClip;
    gridCellBase_
        .set(0, 0, 0, cellLengthX, cellLengthY, boundaryClip.length(2));

    gridCell_.clear();

    // Create grid ordering.
    gridIndex_ = 0;
    grid_.clear();
    editorQueryCreateGrid(grid_, 0, gridXSize_, 0, gridYSize_);
}

bool EditorQuery::nextGrid()
{
    if (gridIndex_ < grid_.size())
    {
        size_t x = grid_[gridIndex_] & 0xfffffU;
        size_t y = (grid_[gridIndex_] >> 20) & 0xfffffU;

        LOG_EDITOR_QUERY("x:" << x << ", y:" << y);

        double dx = static_cast<double>(x) * gridCellBase_.max(0);
        double dy = static_cast<double>(y) * gridCellBase_.max(1);

        gridCell_.set(gridBoundary_.min(0) + dx,
                      gridBoundary_.min(1) + dy,
                      gridBoundary_.min(2),
                      gridBoundary_.min(0) + dx + gridCellBase_.max(0),
                      gridBoundary_.min(1) + dy + gridCellBase_.max(1),
                      gridBoundary_.min(2) + gridCellBase_.max(2));

        gridIndex_++;

        return true;
    }

    return false;
}

bool EditorQuery::Key::operator<(const Key &rhs) const
{
    return (datasetId < rhs.datasetId) || (pageId < rhs.pageId);
}

std::shared_ptr<EditorPage> EditorQuery::read(size_t dataset, size_t index)
{
    Key nk = {dataset, index};

    auto search = cache_.find(nk);
    if (search != cache_.end())
    {
        // Move found page to top
        for (size_t i = 0; i < lru_.size(); i++)
        {
            if (lru_[i] == search->second)
            {
                if (i > 0)
                {
                    std::shared_ptr<EditorPage> tmp = lru_[i];
                    for (size_t j = i; j > 0; j--)
                    {
                        lru_[j] = lru_[j - 1];
                    }
                    lru_[0] = tmp;
                }

                break;
            }
        }

        return search->second;
    }
    else
    {
        // LRU
        if (lru_.size() > 0)
        {
            size_t idx;

            if (lru_.size() < cacheSizeMax_)
            {
                // Make room for new page
                lru_.resize(lru_.size() + 1);
                idx = lru_.size() - 1;
            }
            else
            {
                // Drop oldest page
                idx = lru_.size() - 1;
                if (lru_[idx]->isModified())
                {
                    lru_[idx]->write();
                }
                Key nkrm = {lru_[idx]->datasetId(), lru_[idx]->pageId()};
                cache_.erase(nkrm);
            }

            // New page is on top
            for (size_t j = idx; j > 0; j--)
            {
                lru_[j] = lru_[j - 1];
            }
        }
        else
        {
            // First page is on top
            lru_.resize(1);
        }

        std::shared_ptr<EditorPage> result;
        result = std::make_shared<EditorPage>(editor_,
                                              this,
                                              nk.datasetId,
                                              nk.pageId);
        cache_[nk] = result;
        lru_[0] = result;

        try
        {
            result->read();
        }
        catch (...)
        {
            // error
        }

        return result;
    }
}
