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

/** @file Segments.hpp */

#ifndef SEGMENTS_HPP
#define SEGMENTS_HPP

// Include std.
#include <unordered_map>
#include <unordered_set>

// Include 3D Forest.
#include <Segment.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Segment List. */
class EXPORT_EDITOR Segments
{
public:
    Segments();

    void clear();
    void setDefault();

    size_t size() const { return segments_.size(); }
    Segment &operator[](size_t index) { return segments_[index]; }
    const Segment &operator[](size_t index) const { return segments_[index]; }
    size_t id(size_t pos) const { return segments_[pos].id; }

    void push_back(const Segment &segment);
    void erase(size_t pos);

    size_t index(size_t id, bool throwException = true) const;
    bool contains(size_t id) const;

    size_t unusedId() const;

    void addTree(size_t id,
                 const std::string &label,
                 const Box<double> &boundary);

    bool updateSelection(const std::unordered_set<size_t> &selectedIds,
                         bool ctrl = false);

    void exportMeshList(const std::string &projectFilePath, double scale) const;
    void importMeshList(const std::string &projectFilePath, double scale);

protected:
    std::vector<Segment> segments_;
    std::unordered_map<size_t, size_t> hashTableId_;

    friend void fromJson(Segments &out, const Json &in, double scale);
    friend void toJson(Json &out, const Segments &in, double scale);
};

void fromJson(Segments &out, const Json &in, double scale);
void toJson(Json &out, const Segments &in, double scale);

#include <WarningsEnable.hpp>

#endif /* SEGMENTS_HPP */
