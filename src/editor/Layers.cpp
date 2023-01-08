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

/** @file Layers.cpp */

#include <Layers.hpp>
#include <Log.hpp>

#define MODULE_NAME "Layers"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

Layers::Layers()
{
    setDefault();
}

void Layers::clear()
{
    LOG_DEBUG_LOCAL();
    layers_.clear();
    hashTableId_.clear();
}

void Layers::setDefault()
{
    LOG_DEBUG_LOCAL();
    size_t id = 0;
    size_t idx = 0;

    layers_.resize(1);
    layers_[idx].set(id, "main", {1.0F, 1.0F, 1.0F});

    hashTableId_.clear();
    hashTableId_[id] = idx;
}

void Layers::push_back(const Layer &layer)
{
    LOG_DEBUG_LOCAL(<< "layer <" << layer << ">");
    size_t id = layer.id();
    size_t idx = layers_.size();

    layers_.push_back(layer);

    hashTableId_[id] = idx;
}

void Layers::erase(size_t i)
{
    LOG_DEBUG_LOCAL();

    if (layers_.size() == 0)
    {
        return;
    }

    size_t key = id(i);
    hashTableId_.erase(key);

    size_t n = layers_.size() - 1;
    for (size_t pos = i; pos < n; pos++)
    {
        layers_[pos] = layers_[pos + 1];
        hashTableId_[layers_[pos].id()] = pos;
    }
    layers_.resize(n);
}

size_t Layers::unusedId() const
{
    LOG_DEBUG_LOCAL();
    // Return minimum available id value
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (hashTableId_.find(rval) == hashTableId_.end())
        {
            return rval;
        }
    }

    THROW("New layer identifier is not available.");
}

void Layers::setLabel(size_t i, const std::string &label)
{
    LOG_DEBUG_LOCAL(<< "index <" << i << "> label <" << label << ">");
    layers_[i].setLabel(label);
}

void Layers::setColor(size_t i, const Vector3<float> &color)
{
    LOG_DEBUG_LOCAL(<< "index <" << i << "> color <" << color << ">");
    layers_[i].setColor(color);
}

void Layers::read(const Json &in)
{
    LOG_DEBUG_LOCAL();

    clear();

    if (in.contains("layers"))
    {
        size_t i = 0;
        size_t n = in["layers"].array().size();

        layers_.resize(n);

        for (auto const &it : in["layers"].array())
        {
            layers_[i].read(it);

            size_t id = layers_[i].id();

            hashTableId_[id] = i;

            i++;
        }
    }

    // Default
    if (layers_.size() == 0)
    {
        setDefault();
    }
}

Json &Layers::write(Json &out) const
{
    LOG_DEBUG_LOCAL();

    size_t i = 0;

    for (auto const &it : layers_)
    {
        Json &obj = out["layers"][i];
        it.write(obj);
        i++;
    }

    return out;
}
