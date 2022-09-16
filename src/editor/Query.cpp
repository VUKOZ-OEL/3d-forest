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

/** @file Query.cpp */

#include <queue>

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <Query.hpp>

#define LOG_QUERY(msg)
//#define LOG_QUERY(msg) LOG_MODULE("Query", msg)

Query::Query(Editor *editor) : editor_(editor)
{
    maximumResults_ = 0;
    cacheSizeMax_ = 200;
}

Query::~Query()
{
}

void Query::exec()
{
    selectedPages_.clear();

    if (!selectBox_.empty())
    {
        editor_->datasets().select(selectedPages_, selectBox_);
    }
    if (!selectCone_.empty())
    {
        editor_->datasets().select(selectedPages_, selectCone_.box());
    }
    if (!selectedSphere_.empty())
    {
        editor_->datasets().select(selectedPages_, selectedSphere_.box());
    }

    reset();

    setState(Page::STATE_SELECT);

    nResults_ = 0;
}

void Query::exec(const std::vector<IndexFile::Selection> &selectedPages)
{
    selectedPages_ = selectedPages;

    reset();

    setState(Page::STATE_SELECT);

    nResults_ = 0;
}

void Query::reset()
{
    pageIndex_ = 0;
    pagePointIndex_ = 0;
    pagePointIndexMax_ = 0;
}

void Query::clear()
{
    cache_.clear();
    lru_.clear();

    page_.reset();

    selectedPages_.clear();

    reset();
}

void Query::addResults(size_t n)
{
    if (maximumResults_ > 0)
    {
        nResults_ += n;
    }
}

bool Query::nextPage()
{
    LOG_QUERY("");

    // Reset point index within active page.
    pagePointIndex_ = 0;
    pagePointIndexMax_ = 0;

    if ((nResults_ != 0) && (nResults_ == maximumResults_))
    {
        return false;
    }

    // Find next page in selection.
    while (pageIndex_ < selectedPages_.size())
    {
        LOG_QUERY("pageIndex " << pageIndex_ << "/" << selectedPages_.size());
        IndexFile::Selection &selectedPage = selectedPages_[pageIndex_];
        page_ = read(selectedPage.id, selectedPage.idx);
        page_->nextState();
        pageIndex_++;

        if (page_.get() && page_->selectionSize > 0)
        {
            // This page is in selection.

            // Set point index range within the page.
            pagePointIndexMax_ = page_->selectionSize - 1;

            // Point to current page data.
            position_ = page_->position.data();
            intensity_ = page_->intensity.data();
            returnNumber_ = page_->returnNumber.data();
            numberOfReturns_ = page_->numberOfReturns.data();
            classification_ = page_->classification.data();
            userData_ = page_->userData.data();
            gpsTime_ = page_->gpsTime.data();
            color_ = page_->color.data();

            layer_ = page_->layer.data();
            elevation_ = page_->elevation.data();
            customColor_ = page_->customColor.data();
            descriptor_ = page_->descriptor.data();
            density_ = page_->density.data();
            normal_ = page_->normal.data();
            value_ = page_->value.data();

            selection_ = page_->selection.data();

            return true;
        }
    }

    // There are no more pages in selection.
    return false;
}

size_t Query::pageSizeEstimate() const
{
    return selectedPages_.size();
}

void Query::setModified()
{
    page_->setModified();
}

void Query::flush()
{
    for (size_t i = 0; i < lru_.size(); i++)
    {
        if (lru_[i]->isModified())
        {
            lru_[i]->write();
        }
    }
}

void Query::setState(Page::State state)
{
    for (auto &it : cache_)
    {
        it.second->setState(state);
    }
}

bool Query::nextState()
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

void Query::selectBox(const Box<double> &box)
{
    selectBox_ = box;
}

void Query::selectCone(double x, double y, double z, double z2, double angle)
{
    selectCone_.set(x, y, z, z2, angle);
}

void Query::selectSphere(double x, double y, double z, double radius)
{
    selectedSphere_.set(x, y, z, radius);
}

void Query::selectElevationRange(const Range<double> &elevationRange)
{
    elevationRange_ = elevationRange;
}

void Query::selectClassifications(const std::unordered_set<size_t> &list)
{
    if (list.empty())
    {
        selectClassifications_.clear();
    }
    else
    {
        size_t n = 256;

        selectClassifications_.resize(n);

        for (size_t i = 0; i < n; i++)
        {
            selectClassifications_[i] = 0;
        }

        for (auto const &it : list)
        {
            selectClassifications_[it] = 1;
        }
    }
}

void Query::selectLayers(const std::unordered_set<size_t> &list)
{
    selectLayers_ = list;
}

void Query::selectCamera(const Camera &camera)
{
    double eyeX = camera.eye[0];
    double eyeY = camera.eye[1];
    double eyeZ = camera.eye[2];

    std::vector<std::shared_ptr<Page>> viewPrev;
    viewPrev = lru_;

    lru_.clear();

    std::multimap<double, Key> queue;

    for (size_t i = 0; i < editor_->datasets().size(); i++)
    {
        const Dataset &db = editor_->datasets().at(i);
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

        const Dataset &db = editor_->datasets().key(nk.datasetId);
        const IndexFile::Node *node;
        const IndexFile &index = db.index();
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
                const Page *lru = viewPrev[viewPrev.size() - 1].get();
                Key nkrm = {lru->datasetId(), lru->pageId()};
                cache_.erase(nkrm);
                viewPrev.resize(viewPrev.size() - 1);
            }

            std::shared_ptr<Page> page;
            page =
                std::make_shared<Page>(editor_, this, nk.datasetId, nk.pageId);
            cache_[nk] = page;
            lru_.push_back(page);
        }

        for (size_t i = 0; i < 8; i++)
        {
            if (node->next[i])
            {
                const IndexFile::Node *sub = index.at(node->next[i]);
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

    setState(Page::STATE_RENDER);
}

// Create Z-order (Morton space filling curve), linear order of a quadtree.
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

void Query::setMaximumResults(size_t nPoints)
{
    maximumResults_ = nPoints;
}

void Query::setGrid(size_t pointsPerCell, double cellLengthMinPct)
{
    LOG_QUERY("");

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
    LOG_QUERY("nCells:" << nCells);

    double areaPerCell = areaClip / nCells;
    double cellLength = sqrt(areaPerCell);

    double cellLengthMin = boundaryClip.length(0);
    if (boundaryClip.length(1) < cellLengthMin)
    {
        cellLengthMin = boundaryClip.length(1);
    }
    cellLengthMin = cellLengthMin * 0.01 * cellLengthMinPct;
    if (cellLength < cellLengthMin)
    {
        cellLength = cellLengthMin;
    }

    gridXSize_ =
        static_cast<size_t>(round(boundaryClip.length(0) / cellLength));
    LOG_QUERY("gridXSize_:" << gridXSize_);
    gridYSize_ =
        static_cast<size_t>(round(boundaryClip.length(1) / cellLength));
    LOG_QUERY("gridYSize_:" << gridYSize_);

    double cellLengthX =
        boundaryClip.length(0) / static_cast<double>(gridXSize_);
    LOG_QUERY("cellLengthX:" << cellLengthX);
    double cellLengthY =
        boundaryClip.length(1) / static_cast<double>(gridYSize_);
    LOG_QUERY("cellLengthY:" << cellLengthY);

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

bool Query::nextGrid()
{
    if (gridIndex_ < grid_.size())
    {
        size_t x = grid_[gridIndex_] & 0xfffffU;
        size_t y = (grid_[gridIndex_] >> 20) & 0xfffffU;

        LOG_QUERY("x:" << x << ", y:" << y);

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

bool Query::Key::operator<(const Key &rhs) const
{
    return (datasetId < rhs.datasetId) || (pageId < rhs.pageId);
}

std::shared_ptr<Page> Query::read(size_t dataset, size_t index)
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
                    std::shared_ptr<Page> tmp = lru_[i];
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

        std::shared_ptr<Page> result;
        result = std::make_shared<Page>(editor_, this, nk.datasetId, nk.pageId);
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
