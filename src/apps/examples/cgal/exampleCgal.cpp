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

/** @file exampleCgal.cpp @brief CGAL example. */

// Include Std.
#include <vector>

// Include 3rd party.
#define CGAL_DISABLE_GMP 1
#include <CGAL/Alpha_shape_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_3.h>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleCgal"
#include <Log.hpp>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Alpha_shape_vertex_base_3<K> Vb;
typedef CGAL::Alpha_shape_cell_base_3<K> Cb;
typedef CGAL::Triangulation_data_structure_3<Vb, Cb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Triangulation_3;
typedef CGAL::Alpha_shape_3<Triangulation_3> Alpha_shape_3;
typedef K::Point_3 Point;

static void alphaShape()
{
    // Define a vector of 3D points used to construct the alpha shape
    std::vector<Point> points = {Point(0, 0, 0),
                                 Point(1, 0, 0),
                                 Point(0, 1, 0),
                                 Point(0, 0, 1),
                                 Point(1, 1, 1)};

    // Create an Alpha_shape_3 object from the given points
    // The third parameter '0' is the initial alpha value
    // The fourth parameter specifies the mode
    //   (GENERAL allows for all types of shapes)
    Alpha_shape_3 as(points.begin(), points.end(), 0, Alpha_shape_3::GENERAL);

    // Attempt to find the optimal alpha value that yields a single connected
    // component.
    Alpha_shape_3::Alpha_iterator opt = as.find_optimal_alpha(1);

    // If no valid alpha was found, output an error message and exit
    if (opt == as.alpha_end())
    {
        std::cerr << "No valid alpha found for requested component count!\n";
        return;
    }

    // Set the alpha value of the alpha shape to the optimal one found
    as.set_alpha(*opt);

    // Print the selected alpha value to standard output
    std::cout << "Alpha: " << as.get_alpha() << std::endl;

    // Initialize a counter for surface facets
    int count = 0;

    // Iterate through all facets classified as part of the alpha shape surface
    for (auto it = as.alpha_shape_facets_begin();
         it != as.alpha_shape_facets_end();
         ++it)
    {
        count++; // Increment the count for each surface facet
    }

    // Print the total number of surface facets found
    std::cout << "Number of surface facets: " << count << "\n";

    // Output:
    //   Alpha: 0.75
    //   Number of surface facets: 6

    // Vector to store vertices of all surface triangles
    // (each triangle consists of 3 Point objects)
    std::vector<Point> triangleList;

    // Loop over all facets (triangular faces) that are part of the alpha shape
    for (auto fit = as.alpha_shape_facets_begin();
         fit != as.alpha_shape_facets_end();
         ++fit)
    {
        // Each facet is represented by a pair: a cell (tetrahedron) and the
        // index of the face (0-3) opposite to vertex i
        Alpha_shape_3::Cell_handle cell = fit->first;
        int i = fit->second;

        // Extract the three vertices of the facet by skipping the vertex
        // at index i
        std::array<Point, 3> triangle;
        int idx = 0;
        for (int j = 0; j < 4; ++j)
        {
            if (j != i)
            {
                triangle[idx++] = cell->vertex(j)->point();
            }
        }

        // Store the triangle's vertices sequentially into the triangle list
        triangleList.push_back(triangle[0]);
        triangleList.push_back(triangle[1]);
        triangleList.push_back(triangle[2]);
    }

    // Output all collected triangle vertices
    std::cout << "Triangle vertex list (x y z per point):\n";
    for (size_t i = 0; i < triangleList.size(); ++i)
    {
        const Point &p = triangleList[i];

        // Print coordinates of the current vertex
        std::cout << p.x() << " " << p.y() << " " << p.z() << "\n";

        // After every third point, print a separator to mark the end
        // of one triangle
        if ((i + 1) % 3 == 0)
        {
            std::cout << "---\n"; // triangle separator
        }
    }

    // Output:
    //  Triangle vertex list (x y z per point):
    //  1 0 0
    //  0 0 0
    //  0 0 1
    //  ---
    //  1 1 1
    //  1 0 0
    //  0 0 1
    //  ---
    //  0 0 0
    //  0 0 1
    //  0 1 0
    //  ---
    //  1 1 1
    //  0 0 1
    //  0 1 0
    //  ---
    //  1 0 0
    //  1 1 1
    //  0 1 0
    //  ---
    //  1 0 0
    //  0 0 0
    //  0 1 0
    //  ---
}

int main()
{
    try
    {
        alphaShape();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
