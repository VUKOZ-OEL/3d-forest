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
#include <CDT.h>
#include <convhull_3d.h>

// Include local.
#define LOG_MODULE_NAME "ComputeConvexHullMethod"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ComputeConvexHullMethod::qhull3d(const std::vector<double> &points,
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

void ComputeConvexHullMethod::qhull2d(const std::vector<double> &points,
                                      Mesh &mesh,
                                      float z)
{
    // Calculate the 2D convex hull polygon in the XY plane.
    size_t nv = points.size() / 3;
    CH_FLOAT *v;
    v = static_cast<CH_FLOAT *>(malloc(nv * 2 * sizeof(CH_FLOAT)));
    for (size_t i = 0; i < nv; i++)
    {
        v[i * 2 + 0] = points[i * 3 + 0];
        v[i * 2 + 1] = points[i * 3 + 1];
    }

    int *f = NULL;
    int nFacesOut = 0;

    convhull_nd_build(v, static_cast<int>(nv), 2, &f, NULL, NULL, &nFacesOut);

    // Reduce the original points to the minimal set of points used in the hull.
    size_t nEdges = static_cast<size_t>(nFacesOut);
    unsigned int nVertices = 0;

    CDT::EdgeVec e2d;
    e2d.resize(nEdges);

    std::vector<CDT::V2d<double>> v2d;
    v2d.resize(nEdges);

    std::map<unsigned int, unsigned int> map;

    for (size_t i = 0; i < nEdges; i++)
    {
        // v1
        unsigned int index1 = static_cast<unsigned int>(f[i * 2 + 0]);
        auto it1 = map.find(index1);
        if (it1 != map.end())
        {
            index1 = it1->second;
        }
        else
        {
            v2d[nVertices].x = v[index1 * 2 + 0];
            v2d[nVertices].y = v[index1 * 2 + 1];
            map[index1] = nVertices;
            index1 = nVertices;
            nVertices++;
        }

        // v2
        unsigned int index2 = static_cast<unsigned int>(f[i * 2 + 1]);
        auto it2 = map.find(index2);
        if (it2 != map.end())
        {
            index2 = it2->second;
        }
        else
        {
            v2d[nVertices].x = v[index2 * 2 + 0];
            v2d[nVertices].y = v[index2 * 2 + 1];
            map[index2] = nVertices;
            index2 = nVertices;
            nVertices++;
        }

        // e(v1, v2)
        e2d[i] = CDT::Edge(index1, index2);
    }

    free(v);
    free(f);

    // Calculate Constrained Delaunay Triangulation (CDT).
    CDT::Triangulation<double> cdt;

    cdt.insertVertices(v2d);
    cdt.insertEdges(e2d);
    cdt.eraseOuterTrianglesAndHoles();

    size_t nTriangles = cdt.triangles.size();

    mesh.clear();
    mesh.mode = Mesh::Mode::MODE_TRIANGLES;
    mesh.position.resize(nTriangles * 3 * 3);

    for (size_t i = 0; i < nTriangles; i++)
    {
        size_t A = cdt.triangles[i].vertices[0];
        size_t B = cdt.triangles[i].vertices[1];
        size_t C = cdt.triangles[i].vertices[2];

        mesh.position[i * 9 + 0] = static_cast<float>(v2d[A].x);
        mesh.position[i * 9 + 1] = static_cast<float>(v2d[A].y);
        mesh.position[i * 9 + 2] = z;

        mesh.position[i * 9 + 3] = static_cast<float>(v2d[B].x);
        mesh.position[i * 9 + 4] = static_cast<float>(v2d[B].y);
        mesh.position[i * 9 + 5] = z;

        mesh.position[i * 9 + 6] = static_cast<float>(v2d[C].x);
        mesh.position[i * 9 + 7] = static_cast<float>(v2d[C].y);
        mesh.position[i * 9 + 8] = z;
    }
}
