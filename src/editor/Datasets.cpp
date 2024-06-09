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

// Include 3D Forest.
#include <Datasets.hpp>

// Include local.
#define LOG_MODULE_NAME "Datasets"
#include <Log.hpp>

Datasets::Datasets()
{
    clear();
}

void Datasets::setLabel(size_t i, const std::string &label)
{
    datasets_[i].setLabel(label);
}

void Datasets::setColor(size_t i, const Vector3<double> &color)
{
    datasets_[i].setColor(color);
}

void Datasets::setTranslation(size_t i, const Vector3<double> &translation)
{
    datasets_[i].setTranslation(translation);
}

Box<double> Datasets::boundary(const QueryFilterSet &datasetFilter) const
{
    Box<double> box;

    box.clear();

    for (auto const &it : datasets_)
    {
        if (datasetFilter.contains(it.id()))
        {
            box.extend(it.boundary());
        }
    }

    return box;
}

void Datasets::updateBoundary()
{
    boundary_.clear();

    for (auto const &it : datasets_)
    {
        boundary_.extend(it.boundary());
    }
}

uint64_t Datasets::nPoints() const
{
    uint64_t n = 0;

    for (auto const &it : datasets_)
    {
        n += it.nPoints();
    }

    return n;
}

uint64_t Datasets::nPoints(const QueryFilterSet &datasetFilter) const
{
    uint64_t n = 0;

    for (auto const &it : datasets_)
    {
        if (datasetFilter.contains(it.id()))
        {
            n += it.nPoints();
        }
    }

    return n;
}

void Datasets::selectPages(const QueryFilterSet &datasetFilter,
                           const Box<double> &box,
                           std::vector<IndexFile::Selection> &selected) const
{
    for (auto const &it : datasets_)
    {
        if (datasetFilter.contains(it.id()))
        {
            it.index().selectNodes(selected, box, it.id());
        }
    }
}

void Datasets::clear()
{
    datasets_.resize(0);
    hashTable_.clear();
    datasetsIds_.clear();
    boundary_.clear();
}

size_t Datasets::unusedId() const
{
    // Return minimum available id value.
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTable_.find(rval) == hashTable_.end())
        {
            return rval;
        }
    }

    THROW("New data set identifier is not available.");
}

void Datasets::erase(size_t i)
{
    if (datasets_.size() == 0)
    {
        return;
    }

    size_t key = id(i);
    datasetsIds_.erase(key);

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

void Datasets::read(const std::string &path,
                    const std::string &projectPath,
                    const SettingsImport &settings,
                    const Box<double> &projectBoundary,
                    QueryFilterSet &filter)
{
    Dataset ds;
    size_t id = unusedId();

    ds.read(id, path, projectPath, settings, projectBoundary);

    hashTable_[id] = datasets_.size();
    datasets_.push_back(ds);
    datasetsIds_.insert(id);

    filter.setFilter(id, true);

    updateBoundary();
}

void fromJson(Datasets &out,
              const Json &in,
              const std::string &projectPath,
              QueryFilterSet &filter)
{
    size_t i;
    size_t n;

    i = 0;
    n = in.array().size();
    out.datasets_.resize(n);
    out.hashTable_.clear();

    for (auto const &it : in.array())
    {
        fromJson(out.datasets_[i], it, projectPath);
        out.hashTable_[out.datasets_[i].id()] = i;
        out.datasetsIds_.insert(out.datasets_[i].id());
        filter.setFilter(out.datasets_[i].id(), true);
        i++;
    }
}

void toJson(Json &out, const Datasets &in)
{
    size_t i = 0;

    for (auto const &it : in.datasets_)
    {
        toJson(out[i], it);
        i++;
    }
}

std::ostream &operator<<(std::ostream &out, const Datasets &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
