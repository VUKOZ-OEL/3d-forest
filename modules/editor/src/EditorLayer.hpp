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

/**
    @file EditorLayer.hpp
*/

#ifndef EDITOR_LAYER_HPP
#define EDITOR_LAYER_HPP

#include <Json.hpp>
#include <string>

/** Editor Layer. */
class EditorLayer
{
public:
    // Stored
    std::string label;
    size_t id;
    bool visible;

    EditorLayer();
    ~EditorLayer();

    void read(const Json &in);
    Json &write(Json &out) const;
};

#endif /* EDITOR_LAYER_HPP */
