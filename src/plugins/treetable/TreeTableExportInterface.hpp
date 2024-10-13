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

/** @file TreeTableExportInterface.hpp */

#ifndef TREE_TABLE_EXPORT_INTERFACE_HPP
#define TREE_TABLE_EXPORT_INTERFACE_HPP

// Include 3D Forest.
#include <Segment.hpp>
#include <TreeTableExportProperties.hpp>

/** Tree Table Export Interface. */
class TreeTableExportInterface
{
public:
    virtual ~TreeTableExportInterface() = default;

    virtual bool open() = 0;
    virtual void create(const std::string &path) = 0;
    virtual void write(const Segment &segment) = 0;
    virtual void close() = 0;

    void setProperties(const TreeTableExportProperties &prop)
    {
        properties_ = prop;
    }
    const TreeTableExportProperties &properties() const { return properties_; }

private:
    TreeTableExportProperties properties_;
};

#endif /* TREE_TABLE_EXPORT_INTERFACE_HPP */
