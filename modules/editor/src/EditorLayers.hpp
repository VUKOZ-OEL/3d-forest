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

/** @file EditorLayers.hpp */

#ifndef EDITOR_LAYERS_HPP
#define EDITOR_LAYERS_HPP

#include <EditorLayer.hpp>
#include <unordered_set>

/** Editor Layers. */
class EditorLayers
{
public:
    EditorLayers();

    void clear();

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool b);

    size_t size() const { return layers_.size(); }

    size_t id(size_t i) const { return layers_[i].id(); }

    bool isIdEnabled(size_t id) const
    {
        return idHashTable_.find(id) != idHashTable_.end();
    }
    bool isEnabled(size_t i) const { return layers_[i].isEnabled(); }
    void setEnabled(size_t i, bool b);
    void setEnabledAll(bool b);
    void setInvertAll();

    const std::string &label(size_t i) const { return layers_[i].label(); }
    void setLabel(size_t i, const std::string &label);

    const Vector3<float> &color(size_t i) const { return layers_[i].color(); }
    void setColor(size_t i, const Vector3<float> &color);

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    std::vector<EditorLayer> layers_;
    std::unordered_set<size_t> idHashTable_;
    bool enabled_;
};

#endif /* EDITOR_LAYERS_HPP */
