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

/** @file Mesh.cpp */

// Include std.
#include <cmath>

// Include 3D Forest.
#include <Mesh.hpp>
#include <Vector3.hpp>

// Include local.
#define LOG_MODULE_NAME "Mesh"
#include <Log.hpp>

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::clear()
{
    xyz.clear();
    rgb.clear();
    normal.clear();

    indices.clear();
}

void Mesh::calculateNormals()
{
    if (indices.empty())
    {
        if (mode == Mesh::MODE_TRIANGLES)
        {
            calculateNormalsTriangles();
        }
    }
}

void Mesh::calculateNormalsTriangles()
{
    Vector3<float> v1;
    Vector3<float> v2;
    Vector3<float> vnormal;

    normal.resize(xyz.size());

    size_t nTriangles = xyz.size() / 9;

    for (size_t i = 0; i < nTriangles; i++)
    {
        v1[0] = xyz[i * 9 + 3] - xyz[i * 9 + 0];
        v1[1] = xyz[i * 9 + 4] - xyz[i * 9 + 1];
        v1[2] = xyz[i * 9 + 5] - xyz[i * 9 + 2];

        v2[0] = xyz[i * 9 + 6] - xyz[i * 9 + 0];
        v2[1] = xyz[i * 9 + 7] - xyz[i * 9 + 1];
        v2[2] = xyz[i * 9 + 8] - xyz[i * 9 + 2];

        vnormal = Vector3<float>::crossProduct(v1, v2);
        vnormal.normalize();

        normal[i * 9 + 0] = vnormal[0];
        normal[i * 9 + 1] = vnormal[1];
        normal[i * 9 + 2] = vnormal[2];

        normal[i * 9 + 3] = vnormal[3];
        normal[i * 9 + 4] = vnormal[4];
        normal[i * 9 + 5] = vnormal[5];

        normal[i * 9 + 6] = vnormal[6];
        normal[i * 9 + 7] = vnormal[7];
        normal[i * 9 + 8] = vnormal[8];
    }
}
