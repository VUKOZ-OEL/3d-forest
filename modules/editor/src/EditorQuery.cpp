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

    pagePointIndex_ = 0;
    pagePointIndexMax_ = 0;

    while (pageIndex_ < selectedPages_.size())
    {
        FileIndex::Selection &selectedPage = selectedPages_[pageIndex_];
        page_ = read(selectedPage.id, selectedPage.idx);
        pageIndex_++;

        if (page_.get() && page_->selection.size() > 0)
        {
            pagePointIndexMax_ = page_->selection.size() - 1;

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

    return false;
}

size_t EditorQuery::pageSizeEstimate() const
{
    return selectedPages_.size();
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

void EditorQuery::selectGrid()
{
    gridX_ = 0;
    gridXSize_ = 0;
    gridY_ = 0;
    gridYSize_ = 0;

    uint64_t nPoints = editor_->datasets().nPoints();
    Box<double> boundary = editor_->clipBoundary();
    double area = boundary.length(0) * boundary.length(1);
}

bool EditorQuery::nextGrid()
{
    if (gridX_ < gridXSize_)
    {
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
