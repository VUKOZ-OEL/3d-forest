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

// Include 3D Forest.
#include <QueryFilterSet.hpp>

// Include local.
#define LOG_MODULE_NAME "QueryFilterSet"
#include <Log.hpp>

QueryFilterSet::QueryFilterSet() : enabled_(false)
{
    LOG_DEBUG(<< "Create.");
}

QueryFilterSet::QueryFilterSet(const std::unordered_set<size_t> &list,
                               bool enabled)
    : filter_(list),
      values_(list),
      enabled_(enabled)
{
    LOG_DEBUG(<< "Create.");
}

QueryFilterSet::~QueryFilterSet()
{
    LOG_DEBUG(<< "Destroy.");
}

void QueryFilterSet::clear()
{
    filter_.clear();
    values_.clear();
    enabled_ = false;
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

void QueryFilterSet::setEnabled(size_t id, bool enabled)
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

void fromJson(QueryFilterSet &out, const Json &in)
{
    out.clear();

    fromJson(out.enabled_, in["enabled"]);

    size_t value;

    for (auto const &it : in["filter"].array())
    {
        fromJson(value, it);
        out.filter_.insert(value);
    }

    for (auto const &it : in["values"].array())
    {
        fromJson(value, it);
        out.values_.insert(value);
    }
}

void toJson(Json &out, const QueryFilterSet &in)
{
    toJson(out["enabled"], in.enabled_);

    size_t i = 0;
    for (auto const &it : in.filter_)
    {
        out["filter"][i] = it;
        i++;
    }

    size_t j = 0;
    for (auto const &it : in.values_)
    {
        out["values"][j] = it;
        j++;
    }
}
