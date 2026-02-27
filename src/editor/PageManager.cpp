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

/** @file PageManager.cpp */

// Include 3D Forest.
#include <Editor.hpp>
#include <PageManager.hpp>

// Include local.
#define LOG_MODULE_NAME "PageManager"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

PageManager::PageManager()
{
    LOG_DEBUG(<< "Create.");
}

PageManager::~PageManager()
{
    LOG_DEBUG(<< "Destroy.");
}

bool PageManager::Key::operator<(const Key &rhs) const
{
    if (datasetId != rhs.datasetId)
    {
        return datasetId < rhs.datasetId;
    }

    return pageId < rhs.pageId;
}

std::shared_ptr<PageData> PageManager::readPage(Editor *editor,
                                                size_t dataset,
                                                size_t index)
{
    LOG_DEBUG(<< "Read page <" << index << "> dataset <" << dataset << ">.");

    Key nk = {dataset, index};

    auto search = cache_.find(nk);
    if (search != cache_.end())
    {
        LOG_DEBUG(<< "Return from cache.");
        return search->second;
    }

    std::shared_ptr<PageData> result;
    result = std::make_shared<PageData>(nk.datasetId, nk.pageId);
    cache_[nk] = result;

    try
    {
        LOG_DEBUG(<< "Read new page data.");
        result->readPage(editor);
    }
    catch (...)
    {
        // Some error.
    }

    return result;
}

void PageManager::erasePage(Editor *editor, size_t dataset, size_t index)
{
    LOG_DEBUG(<< "Erase page <" << index << "> dataset <" << dataset << ">.");

    Key nk = {dataset, index};

    auto it = cache_.find(nk);
    if (it != cache_.end())
    {
        if (it->second.use_count() == 1)
        {
            if (it->second->modified())
            {
                it->second->writePage(editor);
            }

            cache_.erase(it);
        }
    }
}
