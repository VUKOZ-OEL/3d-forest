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

/** @file QueryFilterSet.cpp */

#include <Log.hpp>
#include <QueryFilterSet.hpp>

#define MODULE_NAME "QueryFilterSet"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

QueryFilterSet::QueryFilterSet() : enabled_(false)
{
    LOG_DEBUG_LOCAL();
}

QueryFilterSet::QueryFilterSet(const std::unordered_set<size_t> &list)
    : filter_(list),
      values_(list),
      enabled_(false)
{
    LOG_DEBUG_LOCAL();
}

QueryFilterSet::~QueryFilterSet()
{
}

void QueryFilterSet::setFilter(const std::unordered_set<size_t> &list)
{
    filter_ = list;
}

void QueryFilterSet::setValues(const std::unordered_set<size_t> &list)
{
    values_ = list;

    std::unordered_set<size_t> tmp;

    for (auto const &it : filter_)
    {
        if (values_.count(it) > 0)
        {
            tmp.insert(it);
        }
    }

    filter_ = tmp;
}

void QueryFilterSet::none()
{
    filter_.clear();
}

void QueryFilterSet::all()
{
    filter_ = values_;
}

void QueryFilterSet::invert()
{
    std::unordered_set<size_t> tmp;

    for (auto const &it : values_)
    {
        if (filter_.count(it) == 0)
        {
            tmp.insert(it);
        }
    }

    filter_ = tmp;
}

void QueryFilterSet::erase(size_t id)
{
    if (filter_.count(id) > 0)
    {
        filter_.erase(id);
    }

    if (values_.count(id) > 0)
    {
        values_.erase(id);
    }
}

void QueryFilterSet::setFilter(size_t id, bool enabled)
{
    if (enabled)
    {
        if (filter_.count(id) == 0)
        {
            filter_.insert(id);
        }
    }
    else
    {
        if (filter_.count(id) > 0)
        {
            filter_.erase(id);
        }
    }

    if (values_.count(id) == 0)
    {
        values_.insert(id);
    }
}
