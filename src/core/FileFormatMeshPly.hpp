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

/** @file FileFormatMeshPly.hpp */

#ifndef FILE_FORMAT_MESH_PLY_HPP
#define FILE_FORMAT_MESH_PLY_HPP

// Include std.

// Include 3D Forest.
#include <Mesh.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** File Format Mesh PLY. */
class EXPORT_CORE FileFormatMeshPly
{
public:
    static void read(Mesh &mesh, const std::string &path, double scale = 1.0);
    static void write(const Mesh &mesh, const std::string &path, double scale = 1.0);
};

#include <WarningsEnable.hpp>

#endif /* FILE_FORMAT_MESH_PLY_HPP */
