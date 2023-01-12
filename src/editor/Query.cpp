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

#define LOG_MODULE_NAME "Query"

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
    LOG_DEBUG(<< "Called.");
    selectedPages_.clear();

    bool selected = false;

    if (!where().box().empty())
    {
        editor_->datasets().selectPages(where_.dataset(),
                                        where().box(),
                                        selectedPages_);
        selected = true;
    }

    if (!where().cone().empty())
    {
        editor_->datasets().selectPages(where_.dataset(),
                                        where().cone().box(),
                                        selectedPages_);
        selected = true;
    }

    if (!where().sphere().empty())
    {
        LOG_DEBUG(<< "Exec sphere <" << where().sphere().box() << ">.");
        editor_->datasets().selectPages(where_.dataset(),
                                        where().sphere().box(),
                                        selectedPages_);
        selected = true;
    }

    if (!selected)
    {
        editor_->datasets().selectPages(where_.dataset(),
                                        editor_->clipBoundary(),
                                        selectedPages_);
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
    LOG_DEBUG(<< "Number of pages <" << selectedPages_.size() << ">.");

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
        LOG_DEBUG(<< "Current pageIndex <" << pageIndex_ << "/"
                  << selectedPages_.size() << ">.");
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
            lru_[i]->writePage();
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
    LOG_DEBUG(<< "Lru size <" << lru_.size() << ">.");

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

void Query::applyCamera(const Camera &camera)
{
    double eyeX = camera.eye[0];
    double eyeY = camera.eye[1];
    double eyeZ = camera.eye[2];

    std::vector<std::shared_ptr<Page>> viewPrev;
    viewPrev = lru_;

    lru_.clear();

    std::multimap<double, Key> queue;

    if (where().dataset().isFilterEnabled())
    {
        const std::unordered_set<size_t> &idList = where().dataset().filter();
        for (auto const &it : idList)
        {
            queue.insert({0, {it, 0}});
        }
    }
    else
    {
        const std::unordered_set<size_t> &idList = editor_->datasets().idList();
        for (auto const &it : idList)
        {
            queue.insert({0, {it, 0}});
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
    LOG_DEBUG(<< "Called.");

    // Calculate grid cell size.
    uint64_t pointsPerArea = editor_->datasets().nPoints(where().dataset());
    Box<double> boundary = editor_->boundary();
    double area = boundary.length(0) * boundary.length(1);

    Box<double> boundaryClip = editor_->clipBoundary();
    double areaClip = boundaryClip.length(0) * boundaryClip.length(1);
    double areaRatio = areaClip / area;
    double pointsPerAreaClip = static_cast<double>(pointsPerArea) * areaRatio;
    double nCells = pointsPerAreaClip / static_cast<double>(pointsPerCell);
    nCells = ceil(nCells);
    LOG_DEBUG(<< "Number of cells in grid <" << nCells << ">.");

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
    LOG_DEBUG(<< "Grid X Size <" << gridXSize_ << ">.");
    gridYSize_ =
        static_cast<size_t>(round(boundaryClip.length(1) / cellLength));
    LOG_DEBUG(<< "Grid Y Size <" << gridYSize_ << ">.");

    double cellLengthX =
        boundaryClip.length(0) / static_cast<double>(gridXSize_);
    LOG_DEBUG(<< "Grid CellLengthX <" << cellLengthX << ">.");
    double cellLengthY =
        boundaryClip.length(1) / static_cast<double>(gridYSize_);
    LOG_DEBUG(<< "Grid CellLengthY <" << cellLengthY << ">.");

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

        LOG_DEBUG(<< "Grid x <" << x << "> y <" << y << ">.");

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

void Query::setVoxels(double voxelSize, const Box<double> &region)
{
    voxelRegion_ = region;

    // Compute grid resolution and actual voxel size.
    size_t min = 1;
    size_t max = 999999;

    size_t nx = static_cast<size_t>(round(voxelRegion_.length(0) / voxelSize));
    clamp(nx, min, max);
    voxelSize_[0] = voxelRegion_.length(0) / static_cast<double>(nx);

    size_t ny = static_cast<size_t>(round(voxelRegion_.length(1) / voxelSize));
    clamp(ny, min, max);
    voxelSize_[1] = voxelRegion_.length(1) / static_cast<double>(ny);

    size_t nz = static_cast<size_t>(round(voxelRegion_.length(2) / voxelSize));
    clamp(nz, min, max);
    voxelSize_[2] = voxelRegion_.length(2) / static_cast<double>(nz);

    // Initialize voxel iterator.
    pushVoxel(0, 0, 0, nx, ny, nz);
    voxelTotalCount_ = nx * ny * nz;
    voxelVisitedCount_ = 0;
}

bool Query::nextVoxel()
{
    return nextVoxel(this);
}

bool Query::nextVoxel(Query *query)
{
    // Subdivide grid until next voxel cell 1x1x1 is found.
    while (!voxelStack_.empty())
    {
        // Get next cell to process.
        const Box<size_t> &c = voxelStack_.back();
        size_t x1 = c.min(0);
        size_t y1 = c.min(1);
        size_t z1 = c.min(2);
        size_t x2 = c.max(0);
        size_t y2 = c.max(1);
        size_t z2 = c.max(2);
        size_t dx = x2 - x1;
        size_t dy = y2 - y1;
        size_t dz = z2 - z1;
        voxelStack_.pop_back();

        voxelBox_.set(
            voxelRegion_.min(0) + voxelSize_[0] * static_cast<double>(x1),
            voxelRegion_.min(1) + voxelSize_[1] * static_cast<double>(y1),
            voxelRegion_.min(2) + voxelSize_[2] * static_cast<double>(z1),
            voxelRegion_.min(0) + voxelSize_[0] * static_cast<double>(x2),
            voxelRegion_.min(1) + voxelSize_[1] * static_cast<double>(y2),
            voxelRegion_.min(2) + voxelSize_[2] * static_cast<double>(z2));

        // a) Return voxel cell 1x1x1.
        if (dx == 1 && dy == 1 && dz == 1)
        {
            voxelIndex_[0] = x1;
            voxelIndex_[1] = y1;
            voxelIndex_[2] = z1;

            voxelVisitedCount_++;

            return true;
        }

        query->where().setBox(voxelBox_);
        query->setMaximumResults(1);
        query->exec();
        bool containsPoints = query->next();
        query->setMaximumResults(0);
        if (!containsPoints)
        {
            voxelVisitedCount_ += dx * dy * dz;
            continue;
        }

        // b) Subdivide cell 2x2x2, 2x1x1, etc.
        size_t px;
        size_t py;
        size_t pz;

        if (dx >= dy && dx >= dz)
        {
            px = dx / 2;
            py = dx / 2;
            pz = dx / 2;
        }
        else if (dy >= dx && dy >= dz)
        {
            px = dy / 2;
            py = dy / 2;
            pz = dy / 2;
        }
        else
        {
            px = dz / 2;
            py = dz / 2;
            pz = dz / 2;
        }

        if (x1 + px > x2)
        {
            px = dx;
        }
        if (y1 + py > y2)
        {
            py = dy;
        }
        if (z1 + pz > z2)
        {
            pz = dz;
        }

        // Push sub-cells in reverse order to iteration.
        // Creates linear order of an Octree using Morton space filling curve.
        pushVoxel(x1 + px, y1 + py, z1 + pz, x2, y2, z2);
        pushVoxel(x1, y1 + py, z1 + pz, x1 + px, y2, z2);
        pushVoxel(x1 + px, y1, z1 + pz, x2, y1 + py, z2);
        pushVoxel(x1, y1, z1 + pz, x1 + px, y1 + py, z2);

        pushVoxel(x1 + px, y1 + py, z1, x2, y2, z1 + pz);
        pushVoxel(x1, y1 + py, z1, x1 + px, y2, z1 + pz);
        pushVoxel(x1 + px, y1, z1, x2, y1 + py, z1 + pz);
        pushVoxel(x1, y1, z1, x1 + px, y1 + py, z1 + pz);
    }

    return false;
}

void Query::pushVoxel(size_t x1,
                      size_t y1,
                      size_t z1,
                      size_t x2,
                      size_t y2,
                      size_t z2)
{
    if (x1 != x2 && y1 != y2 && z1 != z2)
    {
        voxelStack_.push_back(Box<size_t>(x1, y1, z1, x2, y2, z2));
    }
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
                    lru_[idx]->writePage();
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
            LOG_DEBUG(<< "Page pageId <" << nk.pageId << ">.");
            result->readPage();
        }
        catch (...)
        {
            // error
        }

        return result;
    }
}
