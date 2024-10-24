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

/** @file ViewerInterface.hpp */

#ifndef VIEWER_INTERFACE_HPP
#define VIEWER_INTERFACE_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Camera.hpp>
class Editor;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Viewer Interface. */
class EXPORT_GUI ViewerInterface
{
public:
    virtual ~ViewerInterface() = default;

    virtual std::vector<Camera> camera(size_t viewportId) const = 0;
    virtual std::vector<Camera> camera() const = 0;

    virtual void updateScene(Editor *editor) = 0;
    virtual void resetScene(Editor *editor, bool resetView) = 0;
};

#include <WarningsEnable.hpp>

#endif /* VIEWER_INTERFACE_HPP */
