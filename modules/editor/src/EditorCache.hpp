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
    @file EditorCache.hpp
*/

#ifndef EDITOR_CACHE_HPP
#define EDITOR_CACHE_HPP

#include <Camera.hpp>
#include <EditorTile.hpp>

class EditorBase;

/** Editor Cache. */
class EditorCache
{
public:
    EditorCache(EditorBase *editor);
    ~EditorCache();

    void clear();
    void reload();
    bool loadStep();
    void updateCamera(const Camera &camera);

    size_t tileSize() const { return lru_.size(); }
    EditorTile &tile(size_t index) { return *lru_[index]; }

protected:
    EditorBase *editor_;

    // Cache
    struct Key
    {
        size_t dataSetId;
        size_t tileId;

        bool operator<(const Key &rhs) const;
    };
    size_t cacheSizeMax_;
    std::map<Key, std::shared_ptr<EditorTile>> cache_;

    // Last Recently Used (LRU)
    std::vector<std::shared_ptr<EditorTile>> lru_;

    void load(size_t idx);
    void resetRendering();
};

#endif /* EDITOR_CACHE_HPP */
