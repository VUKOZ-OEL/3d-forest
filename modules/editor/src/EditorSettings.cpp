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

/** @file EditorSettings.cpp */

#include <EditorSettings.hpp>

EditorSettings::View::View()
    : pointSize_(1.0F),
      fogEnabled_(false),
      pointColor_(1.0F, 1.0F, 1.0F)
{
    colorSourceString_ = {
        "Color",
        "Intensity",
        "Return Number",
        "Number of Returns",
        "Classification",
        // "Layer",
        // "User Color",
    };

    colorSourceEnabled_.resize(colorSourceString_.size());
    for (auto &&it : colorSourceEnabled_)
    {
        it = false;
    }
    colorSourceEnabled_[0] = true;
}

float EditorSettings::View::pointSize() const
{
    return pointSize_;
}

void EditorSettings::View::setPointSize(float size)
{
    pointSize_ = size;
}

bool EditorSettings::View::isFogEnabled() const
{
    return fogEnabled_;
}

void EditorSettings::View::setFogEnabled(bool b)
{
    fogEnabled_ = b;
}

void EditorSettings::View::setPointColor(float r, float g, float b)
{
    pointColor_.set(r, g, b);
}

size_t EditorSettings::View::colorSourceSize() const
{
    return colorSourceString_.size();
}

const char *EditorSettings::View::colorSourceString(size_t id) const
{
    return colorSourceString_[id].c_str();
}

bool EditorSettings::View::isColorSourceEnabled(size_t id) const
{
    return colorSourceEnabled_[id];
}

void EditorSettings::View::setColorSourceEnabled(size_t id, bool v)
{
    colorSourceEnabled_[id] = v;
}

void EditorSettings::setView(const View &view)
{
    view_ = view;
}
