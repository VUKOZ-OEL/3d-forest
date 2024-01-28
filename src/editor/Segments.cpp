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
    segments_[idx].set(id, "main", {1.0, 1.0, 1.0});

    hashTableId_.clear();
    hashTableId_[id] = idx;
}

void Segments::push_back(const Segment &segment)
{
    LOG_DEBUG(<< "Append segment <" << segment << ">.");
    size_t id = segment.id();
    size_t idx = segments_.size();

    segments_.push_back(segment);

    hashTableId_[id] = idx;
}

void Segments::erase(size_t i)
{
    LOG_DEBUG(<< "Erase item <" << i << ">.");

    if (segments_.size() == 0)
    {
        return;
    }

    size_t key = id(i);
    hashTableId_.erase(key);

    size_t n = segments_.size() - 1;
    for (size_t pos = i; pos < n; pos++)
    {
        segments_[pos] = segments_[pos + 1];
        hashTableId_[segments_[pos].id()] = pos;
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

void Segments::setLabel(size_t i, const std::string &label)
{
    LOG_DEBUG(<< "Set label index <" << i << "> label <" << label << ">.");
    segments_[i].setLabel(label);
}

void Segments::setColor(size_t i, const Vector3<double> &color)
{
    LOG_DEBUG(<< "Set color index <" << i << "> color <" << color << ">.");
    segments_[i].setColor(color);
}

void Segments::read(const Json &in)
{
    LOG_DEBUG(<< "Read.");

    clear();

    if (in.contains("segments"))
    {
        size_t i = 0;
        size_t n = in["segments"].array().size();

        segments_.resize(n);

        for (auto const &it : in["segments"].array())
        {
            segments_[i].read(it);

            size_t id = segments_[i].id();

            hashTableId_[id] = i;

            i++;
        }
    }

    // Set default.
    if (segments_.size() == 0)
    {
        setDefault();
    }
}

Json &Segments::write(Json &out) const
{
    LOG_DEBUG(<< "Write.");

    size_t i = 0;

    for (auto const &it : segments_)
    {
        Json &obj = out["segments"][i];
        it.write(obj);
        i++;
    }

    return out;
}
