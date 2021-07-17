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

/** @file EditorDatabases.cpp */

#include <EditorDatabases.hpp>

EditorDatabases::EditorDatabases()
{
}

void EditorDatabases::clear()
{
    database_.clear();
}

void EditorDatabases::setDataSets(const EditorDataSets &dataSets)
{
    size_t n = dataSets.size();
    database_.resize(n);

    for (size_t i = 0; i < n; i++)
    {
        database_[i] = std::make_shared<EditorDatabase>();
        database_[i]->setProperties(dataSets.at(i));
    }

    updateBoundary();
}

void EditorDatabases::push_back(std::shared_ptr<EditorDatabase> &database)
{
    database_.push_back(database);
    updateBoundary();
}

void EditorDatabases::select(std::vector<FileIndex::Selection> &selected,
                             const Aabb<double> &box)
{
    for (auto const &it : database_)
    {
        if (it->properties().isEnabled())
        {
            it->index().selectNodes(selected, box, it->properties().id());
        }
    }
}

void EditorDatabases::updateBoundary()
{
    boundary_.clear();
    boundaryView_.clear();

    for (auto const &it : database_)
    {
        if (it->properties().isEnabled())
        {
            boundary_.extend(it->properties().boundary());
            boundaryView_.extend(it->properties().boundaryView());
        }
    }
}
