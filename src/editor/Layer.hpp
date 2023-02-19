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

/** @file Layer.hpp */

#ifndef LAYER_HPP
#define LAYER_HPP

#include <ExportEditor.hpp>
#include <Json.hpp>
#include <LayerData.hpp>
#include <Mesh.hpp>
#include <Vector3.hpp>

/** Layer. */
class EXPORT_EDITOR Layer
{
public:
    Layer();

    void set(size_t id, const std::string &label, const Vector3<float> &color);

    size_t id() const { return id_; }

    const std::string &label() const { return label_; }
    void setLabel(const std::string &label);

    const Vector3<float> &color() const { return color_; }
    void setColor(const Vector3<float> &color);

    const LayerData &data() const { return data_; }
    void setData(const LayerData &data) { data_ = data; }

    size_t meshSize() const { return meshList_.size(); }
    const Mesh &mesh(size_t index) const { return meshList_[index]; }
    void addMesh(const Mesh &mesh) { meshList_.push_back(mesh); }

    void read(const Json &in);
    Json &write(Json &out) const;

protected:
    // Stored
    std::string label_;
    Vector3<float> color_;
    size_t id_;

    LayerData data_;
    std::vector<Mesh> meshList_;
};

inline std::ostream &operator<<(std::ostream &os, const Layer &obj)
{
    return os << "id <" << obj.id() << "> label <" << obj.label() << "> color <"
              << obj.color() << ">";
}

#endif /* LAYER_HPP */
