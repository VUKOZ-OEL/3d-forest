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

/** @file QueryFilterSet.hpp */

#ifndef QUERY_FILTER_SET_HPP
#define QUERY_FILTER_SET_HPP

#include <cstdint>
#include <unordered_set>

#include <ExportEditor.hpp>

/** Data Query Filter Set. */
class EXPORT_EDITOR QueryFilterSet
{
public:
    QueryFilterSet();
    QueryFilterSet(const std::unordered_set<size_t> &list);
    ~QueryFilterSet();

    void setFilterEnabled(bool enabled) { enabled_ = enabled; }
    bool isFilterEnabled() const { return enabled_; }

    bool contains(size_t value) const
    {
        return (enabled_ == false) || (filter_.count(value) > 0);
    }

    const std::unordered_set<size_t> &filter() const { return filter_; }

    void setFilter(const std::unordered_set<size_t> &list);
    void setValues(const std::unordered_set<size_t> &list);

    void none();
    void all();
    void invert();

    void erase(size_t id);

    void setFilter(size_t id, bool enabled);
    bool hasFilter(size_t id) const { return filter_.count(id) > 0; }

private:
    std::unordered_set<size_t> filter_;
    std::unordered_set<size_t> values_;
    bool enabled_;
};

#endif /* QUERY_FILTER_SET_HPP */
