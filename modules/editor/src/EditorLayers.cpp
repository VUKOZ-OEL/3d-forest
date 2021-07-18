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

/** @file EditorLayers.cpp */

#include <EditorLayers.hpp>

EditorLayers::EditorLayers() : enabled_(false)
{
    setDefault();
}

void EditorLayers::clear()
{
    layers_.clear();
    hashTableId_.clear();
    hashTableEnabledId_.clear();
}

void EditorLayers::setDefault()
{
    size_t id = 0;
    size_t idx = 0;

    layers_.resize(1);
    layers_[idx].set(id, "main", true, {1.0F, 1.0F, 1.0F});

    hashTableId_.clear();
    hashTableId_[id] = idx;

    hashTableEnabledId_.clear();
    hashTableEnabledId_.insert(id);
}

void EditorLayers::setEnabled(bool b)
{
    enabled_ = b;
}

void EditorLayers::push_back(const EditorLayer &layer)
{
    size_t id = layer.id();
    size_t idx = layers_.size();

    layers_.push_back(layer);

    hashTableId_[id] = idx;

    if (layer.isEnabled())
    {
        hashTableEnabledId_.insert(id);
    }
}

void EditorLayers::remove(size_t i)
{
    if (layers_.size() == 0)
    {
        return;
    }

    size_t key = id(i);

    size_t n = layers_.size() - 1;
    for (size_t pos = i; pos < n; pos++)
    {
        layers_[pos] = layers_[pos + 1];
    }
    layers_.resize(n);

    hashTableId_.erase(key);
    hashTableEnabledId_.erase(key);
}

size_t EditorLayers::unusedId() const
{
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

void EditorLayers::setEnabled(size_t i, bool b)
{
    layers_[i].setEnabled(b);

    if (b)
    {
        hashTableEnabledId_.insert(layers_[i].id());
    }
    else
    {
        hashTableEnabledId_.erase(layers_[i].id());
    }
}

void EditorLayers::setEnabledAll(bool b)
{
    for (size_t i = 0; i < layers_.size(); i++)
    {
        layers_[i].setEnabled(b);
    }

    hashTableEnabledId_.clear();
    if (b)
    {
        for (size_t i = 0; i < layers_.size(); i++)
        {
            hashTableEnabledId_.insert(layers_[i].id());
        }
    }
}

void EditorLayers::setInvertAll()
{
    hashTableEnabledId_.clear();
    for (size_t i = 0; i < layers_.size(); i++)
    {
        bool b = !layers_[i].isEnabled();
        layers_[i].setEnabled(b);
        if (b)
        {
            hashTableEnabledId_.insert(layers_[i].id());
        }
    }
}

void EditorLayers::setLabel(size_t i, const std::string &label)
{
    layers_[i].setLabel(label);
}

void EditorLayers::setColor(size_t i, const Vector3<float> &color)
{
    layers_[i].setColor(color);
}

void EditorLayers::read(const Json &in)
{
    clear();

    if (in.contains("enabled"))
    {
        enabled_ = in["enabled"].isTrue();
    }
    else
    {
        enabled_ = true;
    }

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

            if (layers_[i].isEnabled())
            {
                hashTableEnabledId_.insert(id);
            }

            i++;
        }
    }

    // Default
    if (layers_.size() == 0)
    {
        setDefault();
    }
}

Json &EditorLayers::write(Json &out) const
{
    out["enabled"] = enabled_;

    size_t i = 0;

    for (auto const &it : layers_)
    {
        Json &obj = out["layers"][i];
        it.write(obj);
        i++;
    }

    return out;
}
