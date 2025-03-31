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

/** @file Segments.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <Segments.hpp>

// Include local.
#define LOG_MODULE_NAME "Segments"
#include <Log.hpp>

Segments::Segments()
{
    setDefault();
}

void Segments::clear()
{
    LOG_DEBUG(<< "Clear.");
    segments_.clear();
    hashTableId_.clear();
}

void Segments::setDefault()
{
    LOG_DEBUG(<< "Set default.");
    size_t id = 0;
    size_t idx = 0;

    segments_.resize(1);
    segments_[idx].id = id;
    segments_[idx].label = "unsegmented";
    segments_[idx].color = {0.6, 0.6, 0.6};

    hashTableId_.clear();
    hashTableId_[id] = idx;
}

void Segments::push_back(const Segment &segment)
{
    LOG_DEBUG(<< "Append segment <" << segment << ">.");
    size_t id = segment.id;
    size_t idx = segments_.size();

    segments_.push_back(segment);

    hashTableId_[id] = idx;
}

void Segments::erase(size_t pos)
{
    LOG_DEBUG(<< "Erase item <" << pos << ">.");

    if (segments_.size() == 0)
    {
        return;
    }

    size_t key = id(pos);
    hashTableId_.erase(key);

    size_t n = segments_.size() - 1;
    for (size_t i = pos; i < n; i++)
    {
        segments_[i] = segments_[i + 1];
        hashTableId_[segments_[i].id] = i;
    }
    segments_.resize(n);
}

size_t Segments::unusedId() const
{
    LOG_DEBUG(<< "Obtain unused id.");
    // Return minimum available id value.
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTableId_.find(rval) == hashTableId_.end())
        {
            return rval;
        }
    }

    THROW("New segment identifier is not available.");
}

void Segments::addTree(size_t id, const Box<double> &boundary)
{
    Segment segment;

    segment.id = id;
    segment.label = "Tree " + std::to_string(segment.id);
    segment.color =
        ColorPalette::MPN65[segment.id % ColorPalette::MPN65.size()];
    segment.boundary = boundary;

    push_back(segment);
}

void Segments::exportMeshList(const std::string &projectFilePath) const
{
    for (auto const &it : segments_)
    {
        it.exportMeshList(projectFilePath);
    }
}

void Segments::importMeshList(const std::string &projectFilePath)
{
}

void fromJson(Segments &out, const Json &in)
{
    out.clear();

    size_t i = 0;
    size_t n = in.array().size();

    out.segments_.resize(n);

    for (auto const &it : in.array())
    {
        fromJson(out.segments_[i], it);
        size_t id = out.segments_[i].id;
        out.hashTableId_[id] = i;
        i++;
    }

    // Set default.
    if (out.segments_.size() == 0)
    {
        out.setDefault();
    }
}

void toJson(Json &out, const Segments &in)
{
    size_t i = 0;

    for (auto const &it : in.segments_)
    {
        toJson(out[i], it);
        i++;
    }
}
