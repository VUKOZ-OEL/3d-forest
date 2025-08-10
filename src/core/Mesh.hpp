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

/** @file Mesh.hpp */

#ifndef MESH_HPP
#define MESH_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <Json.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Mesh. */
class EXPORT_CORE Mesh
{
public:
    /** Mesh Geometric Primitive Type. */
    enum class EXPORT_CORE Mode
    {
        MODE_UNKNOWN,
        MODE_POINTS,
        MODE_LINES,
        MODE_TRIANGLES
    };

    /// Mesh name.
    std::string name;

    /// Mesh geometric primitive type.
    Mesh::Mode mode{Mesh::Mode::MODE_UNKNOWN};

    /// Mesh position vertex data [x0, y0, z0, x1, y1, ...].
    std::vector<float> position;
    /// Mesh color vertex data [r0, g0, b0, r1, g1, ...].
    std::vector<float> color;
    /// Mesh normal vertex data [nx0, ny0, nz0, nx1, ny1, ...].
    std::vector<float> normal;

    /// Mesh vertex indices.
    std::vector<unsigned int> indices;

    Mesh();
    ~Mesh();

    void clear();

    void calculateNormals();

    double calculateSurfaceArea2d();

    void exportPLY(const std::string &path, double scale) const;
    void importPLY(const std::string &path, double scale);

private:
    void calculateNormalsTriangles();
    double calculateSurfaceArea2dTriangles();
};

std::string EXPORT_CORE toString(const Mesh::Mode &in);
void EXPORT_CORE fromString(Mesh::Mode &out, const std::string &in);

void EXPORT_CORE toJson(Json &out, const Mesh::Mode &in);
void EXPORT_CORE fromJson(Mesh::Mode &out, const Json &in);

void EXPORT_CORE toJson(Json &out, const Mesh &in);
void EXPORT_CORE fromJson(Mesh &out, const Json &in);

#include <WarningsEnable.hpp>

#endif /* MESH_HPP */
