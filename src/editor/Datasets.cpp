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

/** @file Datasets.cpp */

#include <unordered_set>

#include <Datasets.hpp>
#include <Log.hpp>

Datasets::Datasets()
{
    clear();
}

void Datasets::setEnabled(size_t i, bool b)
{
    datasets_[i].setEnabled(b);
}

void Datasets::setEnabledAll(bool b)
{
    for (size_t i = 0; i < datasets_.size(); i++)
    {
        datasets_[i].setEnabled(b);
    }
}

void Datasets::setInvertAll()
{
    for (size_t i = 0; i < datasets_.size(); i++)
    {
        bool b = !datasets_[i].isEnabled();
        datasets_[i].setEnabled(b);
    }
}

void Datasets::setLabel(size_t i, const std::string &label)
{
    datasets_[i].setLabel(label);
}

void Datasets::setColor(size_t i, const Vector3<float> &color)
{
    datasets_[i].setColor(color);
}

void Datasets::setTranslation(size_t i, const Vector3<double> &translation)
{
    datasets_[i].setTranslation(translation);
}

void Datasets::clear()
{
    datasets_.resize(0);
    hashTable_.clear();
    boundary_.clear();
}

void Datasets::erase(size_t i)
{
    if (datasets_.size() > 0)
    {
        size_t n = datasets_.size() - 1;
        for (size_t pos = i; pos < n; pos++)
        {
            datasets_[pos] = datasets_[pos + 1];
        }
        datasets_.resize(n);

        hashTable_.clear();
        n = datasets_.size();
        for (size_t pos = 0; pos < n; pos++)
        {
            hashTable_[datasets_[pos].id()] = pos;
        }
    }
}

size_t Datasets::unusedId() const
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

void Datasets::updateBoundary()
{
    boundary_.clear();

    for (auto const &it : datasets_)
    {
        if (it.isEnabled())
        {
            boundary_.extend(it.boundary());
        }
    }
}

uint64_t Datasets::nPoints() const
{
    uint64_t n = 0;

    for (auto const &it : datasets_)
    {
        if (it.isEnabled())
        {
            n += it.nPoints();
        }
    }

    return n;
}

void Datasets::select(std::vector<IndexFile::Selection> &selected,
                      const Box<double> &box) const
{
    for (auto const &it : datasets_)
    {
        if (it.isEnabled())
        {
            it.index().selectNodes(selected, box, it.id());
        }
    }
}

void Datasets::read(const std::string &path,
                    const std::string &projectPath,
                    const SettingsImport &settings,
                    const Box<double> &projectBoundary)
{
    Dataset ds;
    size_t id = unusedId();

    ds.read(id, path, projectPath, settings, projectBoundary);

    hashTable_[id] = datasets_.size();
    datasets_.push_back(ds);

    updateBoundary();
}

void Datasets::read(const Json &in, const std::string &projectPath)
{
    size_t i;
    size_t n;

    i = 0;
    n = in.array().size();
    datasets_.resize(n);
    hashTable_.clear();

    for (auto const &it : in.array())
    {
        datasets_[i].read(it, projectPath);
        hashTable_[datasets_[i].id()] = i;
        i++;
    }
}

Json &Datasets::write(Json &out) const
{
    size_t i = 0;

    for (auto const &it : datasets_)
    {
        it.write(out[i]);
        i++;
    }

    return out;
}

std::ostream &operator<<(std::ostream &os, const Datasets &obj)
{
    Json json;
    os << obj.write(json).serialize();
    return os;
}
