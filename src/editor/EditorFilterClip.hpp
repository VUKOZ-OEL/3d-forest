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

/** @file EditorFilterClip.hpp */

#ifndef EDITOR_FILTER_CLIP_HPP
#define EDITOR_FILTER_CLIP_HPP

#include <Box.hpp>

/** Editor Clip Filter. */
class EditorFilterClip
{
public:
    /** Editor Clip Filter Type. */
    enum Type
    {
        TYPE_NONE,
        TYPE_BOX
    };

    Box<double> box;
    Box<double> boxView;
    Type enabled;

    EditorFilterClip();
    ~EditorFilterClip();

    void clear();

    void read(const Json &in);
    Json &write(Json &out) const;
};

inline EditorFilterClip::EditorFilterClip()
{
    clear();
}

inline EditorFilterClip::~EditorFilterClip()
{
}

inline void EditorFilterClip::clear()
{
    box.clear();
    enabled = TYPE_NONE;
}

inline void EditorFilterClip::read(const Json &in)
{
    box.read(in["box"]);
    if (in["enabled"].string() == "box")
    {
        enabled = TYPE_BOX;
    }
    else
    {
        enabled = TYPE_NONE;
    }
}

inline Json &EditorFilterClip::write(Json &out) const
{
    box.write(out["box"]);
    if (enabled == TYPE_BOX)
    {
        out["enabled"] = "box";
    }
    else
    {
        out["enabled"] = "none";
    }

    return out;
}

#endif /* EDITOR_FILTER_CLIP_HPP */
