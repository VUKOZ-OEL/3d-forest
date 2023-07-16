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

/** @file Layers.hpp */

#ifndef LAYERS_HPP
#define LAYERS_HPP

#include <unordered_map>

#include <Layer.hpp>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Layer List. */
class EXPORT_EDITOR Layers
{
public:
    Layers();

    void clear();
    void setDefault();

    size_t size() const { return layers_.size(); }
    const Layer &at(size_t i) const { return layers_[i]; }

    void push_back(const Layer &layer);
    void erase(size_t i);

    size_t id(size_t i) const { return layers_[i].id(); }
    size_t index(size_t id) const
    {
        const auto &it = hashTableId_.find(id);
        if (it != hashTableId_.end())
        {
            return it->second;
        }
        THROW("Invalid layer id");
    }

    bool contains(size_t id) const
    {
        const auto &it = hashTableId_.find(id);
        if (it != hashTableId_.end())
        {
            return true;
        }
        return false;
    }

    size_t unusedId() const;

    const std::string &label(size_t i) const { return layers_[i].label(); }
    void setLabel(size_t i, const std::string &label);

    const Vector3<double> &color(size_t i) const { return layers_[i].color(); }
    void setColor(size_t i, const Vector3<double> &color);

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    std::vector<Layer> layers_;
    std::unordered_map<size_t, size_t> hashTableId_;
};

#include <WarningsEnable.hpp>

#endif /* LAYERS_HPP */
