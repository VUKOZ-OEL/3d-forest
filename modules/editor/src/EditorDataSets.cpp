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

void EditorDataSets::setTranslation(size_t i,
                                    const Vector3<double> &translation)
{
    dataSets_[i].setTranslation(translation);
}

void EditorDataSets::clear()
{
    dataSets_.resize(0);
    hashTable_.clear();
}

void EditorDataSets::erase(size_t i)
{
    if (dataSets_.size() > 0)
    {
        size_t key = id(i);

        size_t n = dataSets_.size() - 1;
        for (size_t pos = i; pos < n; pos++)
        {
            dataSets_[pos] = dataSets_[pos + 1];
        }
        dataSets_.resize(n);

        hashTable_.erase(key);
    }
}

size_t EditorDataSets::unusedId() const
{
    // Return minimum available id value
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTable_.find(rval) == hashTable_.end())
        {
            return rval;
        }
    }

    THROW("New data set identifier is not available.");
}

void EditorDataSets::read(const std::string &path,
                          const std::string &projectPath,
                          const EditorSettingsImport &settings,
                          const Aabb<double> &projectBoundary)
{
    EditorDataSet ds;
    size_t id = unusedId();

    ds.read(id, path, projectPath, settings, projectBoundary);

    hashTable_[id] = dataSets_.size();
    dataSets_.push_back(ds);
}

void EditorDataSets::read(const Json &in, const std::string &projectPath)
{
    size_t i;
    size_t n;

    i = 0;
    n = in.array().size();
    dataSets_.resize(n);
    hashTable_.clear();

    for (auto const &it : in.array())
    {
        dataSets_[i].read(it, projectPath);
        hashTable_[dataSets_[i].id()] = i;
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
