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

/** @file EditorQuery.hpp */

#ifndef EDITOR_QUERY_HPP
#define EDITOR_QUERY_HPP

#include <Camera.hpp>
#include <EditorPage.hpp>
#include <unordered_set>

class EditorDatabase;

/** Editor Query. */
class EditorQuery
{
public:
    EditorQuery(EditorDatabase *editor);
    ~EditorQuery();

    void selectBox(const Box<double> &box);
    void selectCamera(const Camera &camera);

    const Box<double> &selectedBox() const { return selectBox_; }

    void exec();
    void reset();
    void clear();

    bool nextPoint();
    EditorPoint *point() { return point_; }

    bool nextPage();
    EditorPage *page() { return page_.get(); }
    size_t pageSizeEstimate() const;

    void setStateRead();
    void setStateSelect();
    void setStateRender();
    bool nextState();

    size_t cacheSize() const { return lru_.size(); }
    EditorPage &cache(size_t index) { return *lru_[index]; }

protected:
    EditorDatabase *editor_;

    Box<double> selectBox_;

    // Current
    EditorPoint *point_;
    std::shared_ptr<EditorPage> page_;

    // Iterator
    size_t pageIndex_;
    size_t pagePointIndex_;
    size_t pagePointIndexMax_;
    std::vector<FileIndex::Selection> selectedPages_;

    // Cache
    struct Key
    {
        size_t datasetId;
        size_t pageId;

        bool operator<(const Key &rhs) const;
    };
    size_t cacheSizeMax_;
    std::map<Key, std::shared_ptr<EditorPage>> cache_;

    // Last Recently Used (LRU)
    std::vector<std::shared_ptr<EditorPage>> lru_;

    std::shared_ptr<EditorPage> read(size_t dataset, size_t index);
};

#endif /* EDITOR_QUERY_HPP */
