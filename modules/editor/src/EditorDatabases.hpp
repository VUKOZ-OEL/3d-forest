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

/** @file EditorDatabases.hpp */

#ifndef EDITOR_DATABASES_HPP
#define EDITOR_DATABASES_HPP

#include <EditorDataSets.hpp>
#include <EditorDatabase.hpp>

/** Editor Databases. */
class EditorDatabases
{
public:
    EditorDatabases();

    void clear();

    void setDataSets(const EditorDataSets &dataSets);
    void pushBack(std::shared_ptr<EditorDatabase> &database);

    void select(std::vector<FileIndex::Selection> &selected,
                const Aabb<double> &box);

    size_t size() const { return database_.size(); }
    const EditorDatabase &at(size_t i) const { return *database_[i]; }
    const EditorDatabase &key(size_t id) const
    {
        const auto &it = hashTable_.find(id);
        if (it != hashTable_.end())
        {
            return *database_[it->second];
        }
        THROW("Invalid database id");
    }

    const Aabb<double> &boundary() const { return boundary_; }
    const Aabb<double> &boundaryView() const { return boundaryView_; }
    void updateBoundary();

protected:
    std::vector<std::shared_ptr<EditorDatabase>> database_;
    std::unordered_map<size_t, size_t> hashTable_;
    Aabb<double> boundary_;
    Aabb<double> boundaryView_;
};

#endif /* EDITOR_DATABASES_HPP */
