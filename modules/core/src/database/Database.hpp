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
    @file Database.hpp
*/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <Aabb.hpp>
#include <Camera.hpp>
#include <DatabaseCell.hpp>
#include <DatabaseDataSet.hpp>
#include <map>
#include <string>
#include <vector>

/** Database. */
class Database
{
public:
    Database();
    ~Database();

    void addDataSet(size_t id, const std::string &path, bool enabled);
    void clear();

    const Aabb<double> &boundary() const { return boundary_; }
    const Aabb<double> &boundaryView() const { return boundaryView_; }

    void select(std::vector<OctreeIndex::Selection> &selection);

    void updateCamera(const Camera &camera, bool interactionFinished);
    bool loadView();

    size_t cellSize() const { return view_.size(); }
    DatabaseCell &cell(size_t i) { return *view_[i]; }
    const DatabaseCell &cell(size_t i) const { return *view_[i]; }

    bool isCached(size_t d, size_t c) const;
    DatabaseCell *get(size_t d, size_t c) const;

protected:
    // Data Sets
    std::map<size_t, std::shared_ptr<DatabaseDataSet>> dataSets_;
    Aabb<double> boundary_;
    Aabb<double> boundaryView_;

    void updateBoundary();

    // Cache
    struct Key
    {
        size_t dataSetId;
        size_t cellId;

        bool operator<(const Key &rhs) const;
    };
    size_t cacheSizeMax_;
    std::map<Key, std::shared_ptr<DatabaseCell>> cache_;

    // View
    std::vector<std::shared_ptr<DatabaseCell>> view_;

    void loadView(size_t idx);
    void resetRendering();
};

#endif /* DATABASE_HPP */
