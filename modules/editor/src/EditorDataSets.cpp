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

/** @file EditorDataSets.cpp */

#include <EditorDataSets.hpp>
#include <unordered_set>

EditorDataSets::EditorDataSets()
{
    clear();
}

void EditorDataSets::setEnabled(size_t i, bool b)
{
    dataSets_[i].setEnabled(b);
}

void EditorDataSets::setEnabledAll(bool b)
{
    for (size_t i = 0; i < dataSets_.size(); i++)
    {
        dataSets_[i].setEnabled(b);
    }
}

void EditorDataSets::setInvertAll()
{
    for (size_t i = 0; i < dataSets_.size(); i++)
    {
        bool b = !dataSets_[i].isEnabled();
        dataSets_[i].setEnabled(b);
    }
}

void EditorDataSets::setLabel(size_t i, const std::string &label)
{
    dataSets_[i].setLabel(label);
}

void EditorDataSets::setColor(size_t i, const Vector3<float> &color)
{
    dataSets_[i].setColor(color);
}

void EditorDataSets::clear()
{
    dataSets_.resize(0);
}

size_t EditorDataSets::unusedId() const
{
    // Collect all ids
    std::unordered_set<size_t> hashTable;
    for (auto &it : dataSets_)
    {
        hashTable.insert(it.id());
    }

    // Return minimum available id value
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTable.find(rval) == hashTable.end())
        {
            return rval;
        }
    }

    THROW("New data set identifier is not available.");
}

void EditorDataSets::read(size_t id,
                          const std::string &path,
                          const std::string &projectPath,
                          const EditorSettingsImport &settings,
                          const Aabb<double> &projectBoundary)
{
    EditorDataSet ds;

    ds.read(id, path, projectPath, settings, projectBoundary);

    dataSets_.push_back(ds);
}

void EditorDataSets::read(const Json &in, const std::string &projectPath)
{
    size_t i;
    size_t n;

    i = 0;
    n = in.array().size();
    dataSets_.resize(n);

    for (auto const &it : in.array())
    {
        dataSets_[i].read(it, projectPath);
        i++;
    }
}

Json &EditorDataSets::write(Json &out) const
{
    size_t i = 0;

    for (auto const &it : dataSets_)
    {
        it.write(out[i]);
        i++;
    }

    return out;
}
