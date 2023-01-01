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

/** @file Classifications.hpp */

#ifndef CLASSIFICATIONS_HPP
#define CLASSIFICATIONS_HPP

#include <string>
#include <unordered_set>
#include <vector>

#include <Class.hpp>
#include <ExportEditor.hpp>
#include <Vector3.hpp>

/** Classification List. */
class EXPORT_EDITOR Classifications
{
public:
    Classifications();

    void clear();

    size_t size() const { return classes_.size(); }
    const std::unordered_set<size_t> &ids() const { return ids_; }

    const std::string &label(size_t i) const { return classes_[i].label; }

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    std::vector<Class> classes_;
    std::unordered_set<size_t> ids_;

    void resize(size_t n);
};

#endif /* CLASSIFICATIONS_HPP */
