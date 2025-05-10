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

/** @file ManagementStatusList.hpp */

#ifndef MANAGEMENT_STATUS_LIST_HPP
#define MANAGEMENT_STATUS_LIST_HPP

// Include std.
#include <unordered_map>

// Include 3D Forest.
#include <Json.hpp>
#include <ManagementStatus.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Management Status List. */
class EXPORT_EDITOR ManagementStatusList
{
public:
    ManagementStatusList();

    void clear();
    void setDefault();

    size_t size() const { return data_.size(); }
    ManagementStatus &operator[](size_t index) { return data_[index]; }
    const ManagementStatus &operator[](size_t index) const
    {
        return data_[index];
    }
    size_t id(size_t pos) const { return data_[pos].id; }

    void push_back(const ManagementStatus &managementStatus);
    void erase(size_t pos);

    size_t index(size_t id, bool throwException = true) const;
    bool contains(size_t id) const;

    std::string labelById(size_t id, bool throwException = true) const;

    size_t unusedId() const;

protected:
    std::vector<ManagementStatus> data_;
    std::unordered_map<size_t, size_t> index_;

    friend void fromJson(ManagementStatusList &out, const Json &in);
    friend void toJson(Json &out, const ManagementStatusList &in);
};

void fromJson(ManagementStatusList &out, const Json &in);
void toJson(Json &out, const ManagementStatusList &in);

std::ostream &operator<<(std::ostream &out, const ManagementStatusList &in);

#include <WarningsEnable.hpp>

#endif /* MANAGEMENT_STATUS_LIST_HPP */
