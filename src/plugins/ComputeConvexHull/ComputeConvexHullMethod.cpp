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

/** @file ComputeConvexHullMethod.cpp */

// Include 3D Forest.
#include <ComputeConvexHullMethod.hpp>
#include <Util.hpp>

// Include 3rd party.
#define CONVHULL_3D_ENABLE
#include <convhull_3d.h>

// Include local.
#define LOG_MODULE_NAME "ComputeConvexHullMethod"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ComputeConvexHullMethod::quickhull(const std::vector<double> &points,
                                        Mesh &mesh)
{
    size_t nVertices = points.size() / 3;
    ch_vertex *vertices;
    vertices = static_cast<ch_vertex *>(malloc(nVertices * sizeof(ch_vertex)));
    for (size_t i = 0; i < nVertices; i++)
    {
        vertices[i].x = points[3 * i + 0];
        vertices[i].y = points[3 * i + 1];
        vertices[i].z = points[3 * i + 2];
    }

    int nVerticesInt = static_cast<int>(nVertices);
    int *indices = NULL;
    int nFacesInt = 0;

    convhull_3d_build(vertices, nVerticesInt, &indices, &nFacesInt);

    size_t nFaces = static_cast<size_t>(nFacesInt);

    mesh.clear();
    mesh.mode = Mesh::Mode::MODE_TRIANGLES;
    mesh.position.resize(nFaces * 3 * 3);

    for (size_t i = 0; i < nFaces; i++)
    {
        mesh.position[i * 9 + 0] =
            static_cast<float>(vertices[indices[i * 3 + 0]].x);
        mesh.position[i * 9 + 1] =
            static_cast<float>(vertices[indices[i * 3 + 0]].y);
        mesh.position[i * 9 + 2] =
            static_cast<float>(vertices[indices[i * 3 + 0]].z);

        mesh.position[i * 9 + 3] =
            static_cast<float>(vertices[indices[i * 3 + 1]].x);
        mesh.position[i * 9 + 4] =
            static_cast<float>(vertices[indices[i * 3 + 1]].y);
        mesh.position[i * 9 + 5] =
            static_cast<float>(vertices[indices[i * 3 + 1]].z);

        mesh.position[i * 9 + 6] =
            static_cast<float>(vertices[indices[i * 3 + 2]].x);
        mesh.position[i * 9 + 7] =
            static_cast<float>(vertices[indices[i * 3 + 2]].y);
        mesh.position[i * 9 + 8] =
            static_cast<float>(vertices[indices[i * 3 + 2]].z);
    }

    mesh.calculateNormals();

    free(vertices);
    free(indices);
}
