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

/** @file SpeciesList.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <SpeciesList.hpp>

// Include local.
#define LOG_MODULE_NAME "SpeciesList"
#include <Log.hpp>

SpeciesList::SpeciesList()
{
}

void SpeciesList::clear()
{
    LOG_DEBUG(<< "Clear.");
    data_.clear();
    index_.clear();
}

void SpeciesList::setDefault()
{
    LOG_DEBUG(<< "Set default.");

    clear();

    push_back(Species(0, "unknown", {0.8, 0.8, 0.8}));
    push_back(Species(1, "Picea abies", {0.5, 1.0, 0.5}));
    push_back(Species(2, "Cedrus sp", {0.5, 0.5, 1.0}));
}

void SpeciesList::push_back(const Species &species)
{
    LOG_DEBUG(<< "Append species <" << species << ">.");

    size_t id = species.id;
    size_t idx = data_.size();

    data_.push_back(species);

    index_[id] = idx;
}

void SpeciesList::erase(size_t pos)
{
    LOG_DEBUG(<< "Erase item <" << pos << ">.");

    if (data_.size() == 0)
    {
        return;
    }

    size_t key = id(pos);
    index_.erase(key);

    size_t n = data_.size() - 1;
    for (size_t i = pos; i < n; i++)
    {
        data_[i] = data_[i + 1];
        index_[data_[i].id] = i;
    }
    data_.resize(n);
}

size_t SpeciesList::index(size_t id, bool throwException) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return it->second;
    }

    if (throwException)
    {
        THROW("Invalid species id");
    }
    else
    {
        return SIZE_MAX;
    }
}

bool SpeciesList::contains(size_t id) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return true;
    }
    return false;
}

size_t SpeciesList::unusedId() const
{
    LOG_DEBUG(<< "Obtain unused id.");
    // Return minimum available id value.
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (index_.find(rval) == index_.end())
        {
            return rval;
        }
    }

    THROW("New species identifier is not available.");
}

void fromJson(SpeciesList &out, const Json &in)
{
    out.clear();

    size_t i = 0;
    size_t n = in.array().size();

    out.data_.resize(n);

    for (auto const &it : in.array())
    {
        fromJson(out.data_[i], it);
        size_t id = out.data_[i].id;
        out.index_[id] = i;
        i++;
    }
}

void toJson(Json &out, const SpeciesList &in)
{
    size_t i = 0;

    for (auto const &it : in.data_)
    {
        toJson(out[i], it);
        i++;
    }
}

std::ostream &operator<<(std::ostream &out, const SpeciesList &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
