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

/** @file EditorLayer.cpp */

#include <EditorLayer.hpp>
#include <Error.hpp>

EditorLayer::EditorLayer() : id_(0), enabled_(true)
{
}

void EditorLayer::set(size_t id,
                      const std::string &label,
                      bool enabled,
                      const Vector3<float> &color)
{
    id_ = id;
    label_ = label;
    enabled_ = enabled;
    color_ = color;
}

void EditorLayer::setEnabled(bool b)
{
    enabled_ = b;
}

void EditorLayer::setLabel(const std::string &label)
{
    label_ = label;
}

void EditorLayer::setColor(const Vector3<float> &color)
{
    color_ = color;
}

void EditorLayer::read(const Json &in)
{
    if (!in.isObject())
    {
        THROW("Layer is not JSON object");
    }

    // ID
    id_ = in["id"].uint32();

    // Enabled
    if (in.contains("enabled"))
    {
        enabled_ = in["enabled"].isTrue();
    }
    else
    {
        enabled_ = true;
    }

    // Label
    if (in.contains("label"))
    {
        label_ = in["label"].string();
    }
    else
    {
        label_ = "";
    }

    // Color
    if (in.contains("color"))
    {
        color_.read(in["color"]);
    }
}

Json &EditorLayer::write(Json &out) const
{
    out["id"] = id_;
    out["label"] = label_;
    out["enabled"] = enabled_;
    color_.write(out["color"]);

    return out;
}
