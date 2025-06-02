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

/** @file ComputeHullMethod.cpp */

// Include 3D Forest.
#include <ComputeHullMethod.hpp>
#include <Util.hpp>

// Include 3rd party.
#define CONVHULL_3D_ENABLE
#include <CDT.h>
#include <convhull_3d.h>

// Fix the compiler error "too many template parameters" in boost when
// including both Alpha_shape_2.h and Alpha_shape_3.h.
#include <boost/math/policies/policy.hpp>

#define CGAL_DISABLE_GMP 1
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Alpha_shape_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_3.h>

#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <CGAL/Polygon_2.h>

// Include local.
#define LOG_MODULE_NAME "ComputeHullMethod"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ComputeHullMethod::qhull3d(const std::vector<double> &points, Mesh &mesh)
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

void ComputeHullMethod::qhull2d(const std::vector<double> &points,
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

// Helper for keying by rounded point position (epsilon ~1e-6)
struct RoundedPointKey
{
    int x, y, z;
    static constexpr double scale = 1e6;

    RoundedPointKey(double x_, double y_, double z_)
    {
        x = static_cast<int>(std::round(x_ * scale));
        y = static_cast<int>(std::round(y_ * scale));
        z = static_cast<int>(std::round(z_ * scale));
    }

    bool operator<(const RoundedPointKey &other) const
    {
        return std::tie(x, y, z) < std::tie(other.x, other.y, other.z);
    }
};

void ComputeHullMethod::alphaShape3(const std::vector<double> &points,
                                    Mesh &mesh,
                                    double alpha)
{
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
    typedef CGAL::Alpha_shape_vertex_base_3<K> Vb;
    typedef CGAL::Alpha_shape_cell_base_3<K> Cb;
    typedef CGAL::Triangulation_data_structure_3<Vb, Cb> Tds;
    typedef CGAL::Delaunay_triangulation_3<K, Tds> Triangulation_3;
    typedef CGAL::Alpha_shape_3<Triangulation_3> Alpha_shape_3;
    typedef K::Point_3 Point;
    typedef K::Triangle_3 Triangle;
    typedef K::Vector_3 Vector;

    typedef Alpha_shape_3::Cell_handle Cell_handle;
    // typedef Alpha_shape_3::Vertex_handle Vertex_handle;
    // typedef Alpha_shape_3::Facet Facet;

    // Define a vector of 3D points used to construct the alpha shape.
    std::vector<Point> pointsAS;

    size_t nPoints = points.size() / 3;
    pointsAS.resize(nPoints);
    for (size_t i = 0; i < nPoints; i++)
    {
        pointsAS[i] =
            Point(points[i * 3 + 0], points[i * 3 + 1], points[i * 3 + 2]);
    }

    // Create an Alpha_shape_3 object from the given points.
    // The third parameter '0' is the initial alpha value.
    // The fourth parameter specifies the mode
    //   (GENERAL allows for all types of shapes).
    Alpha_shape_3 as(pointsAS.begin(),
                     pointsAS.end(),
                     0,
                     Alpha_shape_3::GENERAL);

    // Attempt to find the optimal alpha value.
    if (alpha < 0.0)
    {
        Alpha_shape_3::Alpha_iterator opt = as.find_optimal_alpha(1);

        // If no valid alpha was found, output an error message and exit.
        if (opt == as.alpha_end())
        {
            LOG_ERROR(<< "No valid optimal alpha value found.");
            return;
        }

        as.set_alpha(*opt);
    }
    else
    {
        as.set_alpha(alpha);
    }

    LOG_DEBUG(<< "Using alpha <" << as.get_alpha() << ">.");

    // Flat list of triangle vertices: a, b, c, d, e, f ...
    // Each group of 3 represents a triangle
    std::vector<Point> vertices;

    // Matching flat list of normals: one per vertex in vertices
    std::vector<Vector> normals;

    // Intermediate structures
    std::map<Point, Vector> accumulatedNormals;
    std::map<RoundedPointKey, Vector> normal_buckets;

    // First pass: accumulate normals at each unique vertex
    for (auto fit = as.finite_facets_begin(); fit != as.finite_facets_end();
         ++fit)
    {
        auto type = as.classify(*fit);
        if (type != Alpha_shape_3::REGULAR && type != Alpha_shape_3::SINGULAR)
            continue;

        Cell_handle cell = fit->first;
        int i = fit->second;

        Point p[3];
        int idx = 0;
        for (int j = 0; j < 4; ++j)
            if (j != i)
                p[idx++] = cell->vertex(j)->point();

        Vector normal = CGAL::cross_product(p[1] - p[0], p[2] - p[0]);

        if (normal != CGAL::NULL_VECTOR)
            for (int k = 0; k < 3; ++k)
            {
                RoundedPointKey key(p[k].x(), p[k].y(), p[k].z());
                normal_buckets[key] = normal_buckets[key] + normal;
                accumulatedNormals[p[k]] = accumulatedNormals[p[k]] + normal;
            }
    }

    // Second pass: rebuild flat triangle list and use smoothed normals
    for (auto fit = as.finite_facets_begin(); fit != as.finite_facets_end();
         ++fit)
    {
        auto type = as.classify(*fit);
        if (type != Alpha_shape_3::REGULAR && type != Alpha_shape_3::SINGULAR)
            continue;

        Cell_handle cell = fit->first;
        int i = fit->second;

        Point p[3];
        int idx = 0;
        for (int j = 0; j < 4; ++j)
            if (j != i)
                p[idx++] = cell->vertex(j)->point();

        for (int k = 0; k < 3; ++k)
        {
            vertices.push_back(p[k]);

            // Vector n = accumulatedNormals[p[k]];
            RoundedPointKey key(p[k].x(), p[k].y(), p[k].z());
            Vector n = normal_buckets[key];
            if (n != CGAL::NULL_VECTOR)
                n = n / std::sqrt(n.squared_length());

            normals.push_back(n);
        }
    }

    // Create Mesh.
    size_t nFaces = vertices.size() / 3;

    mesh.clear();
    mesh.mode = Mesh::Mode::MODE_TRIANGLES;
    mesh.position.resize(nFaces * 3 * 3);

    for (size_t i = 0; i < nFaces; i++)
    {
        mesh.position[i * 9 + 0] = static_cast<float>(vertices[i * 3 + 0].x());
        mesh.position[i * 9 + 1] = static_cast<float>(vertices[i * 3 + 0].y());
        mesh.position[i * 9 + 2] = static_cast<float>(vertices[i * 3 + 0].z());

        mesh.position[i * 9 + 3] = static_cast<float>(vertices[i * 3 + 1].x());
        mesh.position[i * 9 + 4] = static_cast<float>(vertices[i * 3 + 1].y());
        mesh.position[i * 9 + 5] = static_cast<float>(vertices[i * 3 + 1].z());

        mesh.position[i * 9 + 6] = static_cast<float>(vertices[i * 3 + 2].x());
        mesh.position[i * 9 + 7] = static_cast<float>(vertices[i * 3 + 2].y());
        mesh.position[i * 9 + 8] = static_cast<float>(vertices[i * 3 + 2].z());
    }
#if 0
    mesh.normal.resize(nFaces * 3 * 3);

    for (size_t i = 0; i < nFaces; i++)
    {
        mesh.normal[i * 9 + 0] = static_cast<float>(normals[i * 3 + 0].x());
        mesh.normal[i * 9 + 1] = static_cast<float>(normals[i * 3 + 0].y());
        mesh.normal[i * 9 + 2] = static_cast<float>(normals[i * 3 + 0].z());

        mesh.normal[i * 9 + 3] = static_cast<float>(normals[i * 3 + 1].x());
        mesh.normal[i * 9 + 4] = static_cast<float>(normals[i * 3 + 1].y());
        mesh.normal[i * 9 + 5] = static_cast<float>(normals[i * 3 + 1].z());

        mesh.normal[i * 9 + 6] = static_cast<float>(normals[i * 3 + 2].x());
        mesh.normal[i * 9 + 7] = static_cast<float>(normals[i * 3 + 2].y());
        mesh.normal[i * 9 + 8] = static_cast<float>(normals[i * 3 + 2].z());
    }

    // mesh.calculateNormals();
#endif
    // Calculate the volume.
    double totalVolume = 0.0;
    for (auto it = as.finite_cells_begin(); it != as.finite_cells_end(); ++it)
    {
        // Skip cells that are not part of the solid interior.
        if (as.classify(it) != Alpha_shape_3::INTERIOR)
        {
            continue;
        }

        // Get the 4 points of the tetrahedron.
        const Point &p0 = it->vertex(0)->point();
        const Point &p1 = it->vertex(1)->point();
        const Point &p2 = it->vertex(2)->point();
        const Point &p3 = it->vertex(3)->point();

        // Compute signed volume of the tetrahedron.
        double volume = std::abs(CGAL::to_double(CGAL::scalar_product(
                            CGAL::cross_product(p1 - p0, p2 - p0),
                            p3 - p0))) /
                        6.0;

        totalVolume += volume;
    }

    mesh.volume = totalVolume;

    // Calculate the surface area.
    double totalArea = 0.0;

    // Loop through all finite facets (triangular faces of tetrahedra).
    for (auto it = as.finite_facets_begin(); it != as.finite_facets_end(); ++it)
    {
        // Only consider facets which are part of the alpha shape boundary.
        if (as.classify(*it) != Alpha_shape_3::REGULAR)
        {
            continue;
        }

        auto cell = it->first;
        int i = it->second;

        // Get neighboring cell across the facet.
        auto neighbor = cell->neighbor(i);

        // Classify the two cells on either side of the facet.
        auto class1 = as.classify(cell);
        auto class2 = as.classify(neighbor);

        // Check if this facet separates INTERIOR from NON-INTERIOR
        // (i.e., lies on the surface).
        bool isBoundary = (class1 == Alpha_shape_3::INTERIOR &&
                           class2 != Alpha_shape_3::INTERIOR) ||
                          (class2 == Alpha_shape_3::INTERIOR &&
                           class1 != Alpha_shape_3::INTERIOR);

        if (isBoundary)
        {
            // The facet is opposite vertex 'i', so the triangle is formed by
            // the other three vertices.
            int i1 = (i + 1) % 4;
            int i2 = (i + 2) % 4;
            int i3 = (i + 3) % 4;

            const Point &p1 = cell->vertex(i1)->point();
            const Point &p2 = cell->vertex(i2)->point();
            const Point &p3 = cell->vertex(i3)->point();

            // Compute the area of the triangle and add to the total.
            Triangle tri(p1, p2, p3);
            totalArea += std::sqrt(CGAL::to_double(tri.squared_area()));
        }
    }

    mesh.surfaceArea = totalArea;
}

void ComputeHullMethod::alphaShape2(const std::vector<double> &points,
                                    Mesh &mesh,
                                    double alpha,
                                    float z)
{
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

    typedef K::Point_2 Point;

    typedef CGAL::Alpha_shape_vertex_base_2<K> Vb;
    typedef CGAL::Alpha_shape_face_base_2<K> Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
    typedef CGAL::Delaunay_triangulation_2<K, Tds> Triangulation_2;
    typedef CGAL::Alpha_shape_2<Triangulation_2> Alpha_shape_2;

    // Define a vector of 2D points used to construct the alpha shape.
    std::vector<Point> pointsAS;

    size_t nPoints = points.size() / 3;
    pointsAS.resize(nPoints);
    for (size_t i = 0; i < nPoints; i++)
    {
        pointsAS[i] = Point(points[i * 3 + 0], points[i * 3 + 1]);
    }

    // Create an Alpha_shape_2 object from the given points.
    Alpha_shape_2 as(pointsAS.begin(),
                     pointsAS.end(),
                     0,
                     Alpha_shape_2::GENERAL);

    // Attempt to find the optimal alpha value.
    if (alpha < 0.0)
    {
        Alpha_shape_2::Alpha_iterator opt = as.find_optimal_alpha(1);

        // If no valid alpha was found, output an error message and exit.
        if (opt == as.alpha_end())
        {
            LOG_ERROR(<< "No valid optimal alpha value found.");
            return;
        }

        as.set_alpha(*opt);
    }
    else
    {
        as.set_alpha(alpha);
    }

    LOG_DEBUG(<< "Using alpha <" << as.get_alpha() << ">.");

    // Extract triangles.
    std::vector<Point> triangles;
    for (auto it = as.finite_faces_begin(); it != as.finite_faces_end(); ++it)
    {
        if (as.classify(it) != Alpha_shape_2::INTERIOR)
        {
            continue;
        }

        for (int i = 0; i < 3; ++i)
        {
            triangles.push_back(it->vertex(i)->point());
        }
    }

    // Create Mesh.
    size_t nFaces = triangles.size() / 3;

    mesh.clear();
    mesh.mode = Mesh::Mode::MODE_TRIANGLES;
    mesh.position.resize(nFaces * 3 * 3);

    for (size_t i = 0; i < nFaces; i++)
    {
        mesh.position[i * 9 + 0] = static_cast<float>(triangles[i * 3 + 0].x());
        mesh.position[i * 9 + 1] = static_cast<float>(triangles[i * 3 + 0].y());
        mesh.position[i * 9 + 2] = z;

        mesh.position[i * 9 + 3] = static_cast<float>(triangles[i * 3 + 1].x());
        mesh.position[i * 9 + 4] = static_cast<float>(triangles[i * 3 + 1].y());
        mesh.position[i * 9 + 5] = z;

        mesh.position[i * 9 + 6] = static_cast<float>(triangles[i * 3 + 2].x());
        mesh.position[i * 9 + 7] = static_cast<float>(triangles[i * 3 + 2].y());
        mesh.position[i * 9 + 8] = z;
    }

    mesh.calculateNormals();

#if 0
    // Extract polygon boundary.
    std::vector<Point> boundary;
    for (Alpha_shape_edges_iterator it = as.alpha_shape_edges_begin();
         it != as.alpha_shape_edges_end();
         ++it)
    {
        auto seg = as.segment(*it);
        boundary.push_back(seg.source());
        boundary.push_back(seg.target());
    }

    // Create Mesh.
    nPoints = boundary.size();

    mesh.clear();
    mesh.mode = Mesh::Mode::MODE_LINES;
    mesh.position.resize(nPoints * 3);

    for (size_t i = 0; i < nPoints; i++)
    {
        mesh.position[i * 3 + 0] = static_cast<float>(boundary[i].x());
        mesh.position[i * 3 + 1] = static_cast<float>(boundary[i].y());
        mesh.position[i * 3 + 2] = z;
    }
#endif
}

double ComputeHullMethod::surface2(const Mesh &mesh)
{
    if (mesh.mode != Mesh::Mode::MODE_LINES)
    {
        LOG_ERROR(<< "Invalid mesh mode <" << toString(mesh.mode) << ">.");
        return 0;
    }

    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
    typedef K::Point_2 Point;
    typedef CGAL::Polygon_2<K> Polygon_2;

    // Build edge map.
    std::multimap<Point, Point> edge_map;
    const auto &position = mesh.position;
    size_t nLines = position.size() / 6;
    for (size_t i = 0; i < nLines; i++)
    {
        float ax = position[i * 6 + 0];
        float ay = position[i * 6 + 1];
        float bx = position[i * 6 + 3];
        float by = position[i * 6 + 4];
        edge_map.insert({Point(ax, ay), Point(bx, by)});
        edge_map.insert({Point(bx, by), Point(ax, ay)}); // Undirected edge.
    }

    // Trace the polygon.
    std::vector<Point> ordered;
    Point start = edge_map.begin()->first;
    Point current = start;
    Point previous;

    do
    {
        ordered.push_back(current);
        auto range = edge_map.equal_range(current);
        Point next;
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second != previous)
            {
                next = it->second;
                break;
            }
        }
        previous = current;
        current = next;
    } while (current != start && ordered.size() < edge_map.size());

    // Compute area.
    Polygon_2 polygon(ordered.begin(), ordered.end());
    double area = std::abs(polygon.area());

    LOG_DEBUG(<< "Calculated area <" << area << ">.");

    return area;
}
