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

/** @file PageManager.hpp */

#ifndef PAGE_MANAGER_HPP
#define PAGE_MANAGER_HPP

// Include 3D Forest.
#include <PageData.hpp>
class Editor;

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Page Manager. */
class EXPORT_EDITOR PageManager
{
public:
    PageManager();
    ~PageManager();

    std::shared_ptr<PageData> get(Editor *editor, size_t dataset, size_t index);
    void erase(Editor *editor, size_t dataset, size_t index);

private:
    struct Key
    {
        size_t datasetId;
        size_t pageId;

        bool operator<(const Key &rhs) const;
    };

    std::map<Key, std::shared_ptr<PageData>> cache_;
};

#include <WarningsEnable.hpp>

#endif /* PAGE_MANAGER_HPP */
