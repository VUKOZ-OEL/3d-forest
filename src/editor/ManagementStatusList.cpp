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

/** @file ManagementStatusList.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <ManagementStatusList.hpp>

// Include local.
#define LOG_MODULE_NAME "ManagementStatusList"
#include <Log.hpp>

ManagementStatusList::ManagementStatusList()
{
}

void ManagementStatusList::clear()
{
    LOG_DEBUG(<< "Clear.");
    data_.clear();
    index_.clear();
}

void ManagementStatusList::setDefault()
{
    LOG_DEBUG(<< "Set default.");

    clear();

    push_back(ManagementStatus(0, "untouched", {1.0, 0.5, 0.5}));
    push_back(ManagementStatus(1, "target tree", {0.5, 1.0, 0.5}));
    push_back(ManagementStatus(2, "remove - concurency", {0.5, 0.5, 1.0}));
    push_back(ManagementStatus(3, "remove - health", {1.0, 1.0, 0.5}));
    push_back(ManagementStatus(4, "remove - target DBH", {0.5, 1.0, 1.0}));
    push_back(ManagementStatus(5, "retain - habitat", {1.0, 0.5, 1.0}));
}

void ManagementStatusList::push_back(const ManagementStatus &managementStatus)
{
    LOG_DEBUG(<< "Append management status <" << managementStatus << ">.");

    size_t id = managementStatus.id;
    size_t idx = data_.size();

    data_.push_back(managementStatus);

    index_[id] = idx;
}

void ManagementStatusList::erase(size_t pos)
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

size_t ManagementStatusList::index(size_t id, bool throwException) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return it->second;
    }

    if (throwException)
    {
        THROW("Invalid management status id");
    }
    else
    {
        return SIZE_MAX;
    }
}

bool ManagementStatusList::contains(size_t id) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return true;
    }
    return false;
}

std::string ManagementStatusList::labelById(size_t id,
                                            bool throwException) const
{
    size_t idx = index(id, throwException);
    if (idx != SIZE_MAX)
    {
        return data_[idx].label;
    }
    return "Unknown";
}

size_t ManagementStatusList::unusedId() const
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

    THROW("New management status identifier is not available.");
}

void fromJson(ManagementStatusList &out, const Json &in)
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

void toJson(Json &out, const ManagementStatusList &in)
{
    size_t i = 0;

    for (auto const &it : in.data_)
    {
        toJson(out[i], it);
        i++;
    }
}

std::ostream &operator<<(std::ostream &out, const ManagementStatusList &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
