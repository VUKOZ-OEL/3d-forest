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
    clear();
}

void EditorLayers::setEnabled(bool b)
{
    enabled_ = b;
}

void EditorLayers::setEnabled(size_t i, bool b)
{
    layers_[i].setEnabled(b);

    if (b)
    {
        idHashTable_.insert(layers_[i].id());
    }
    else
    {
        idHashTable_.erase(layers_[i].id());
    }
}

void EditorLayers::setEnabledAll(bool b)
{
    for (size_t i = 0; i < layers_.size(); i++)
    {
        layers_[i].setEnabled(b);
    }

    idHashTable_.clear();
    if (b)
    {
        for (size_t i = 0; i < layers_.size(); i++)
        {
            idHashTable_.insert(layers_[i].id());
        }
    }
}

void EditorLayers::setInvertAll()
{
    idHashTable_.clear();
    for (size_t i = 0; i < layers_.size(); i++)
    {
        bool b = !layers_[i].isEnabled();
        layers_[i].setEnabled(b);
        if (b)
        {
            idHashTable_.insert(layers_[i].id());
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

void EditorLayers::clear()
{
    layers_.resize(1);
    layers_[0].set(0, "main", true, {1.0F, 1.0F, 1.0F});
}

void EditorLayers::read(const Json &in)
{
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

        clear();
        layers_.resize(n);

        for (auto const &it : in["layers"].array())
        {
            layers_[i].read(it);
            i++;
        }
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
