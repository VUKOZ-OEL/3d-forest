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

/** @file exampleMeshDistance.cpp @brief Mesh distance example. */

// Include 3rd party.
#include <Eigen/Core>
#include <delaunator.hpp>
#include <igl/point_mesh_squared_distance.h>
#include <igl/writeOBJ.h>

// Include 3D Forest.
#include <Error.hpp>
#include <Vector3.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleMeshDistance"
#include <Log.hpp>

static void printMesh(const Eigen::MatrixXd &V, const Eigen::MatrixXi &F)
{
    std::cout << "V " << V.rows() << " x " << V.cols() << " ("
              << V.rowsCapacity() << " x " << V.colsCapacity() << ") data "
              << V.data() << std::endl;

    std::cout << "F " << F.rows() << " x " << F.cols() << " ("
              << F.rowsCapacity() << " x " << F.colsCapacity() << ") data "
              << F.data() << std::endl;

    for (Eigen::Index i = 0; i < F.rows(); i++)
    {
        // Get vertices
        Vector3<double> p1(V(F(i, 0), 0), V(F(i, 0), 1), V(F(i, 0), 2));
        Vector3<double> p2(V(F(i, 1), 0), V(F(i, 1), 1), V(F(i, 1), 2));
        Vector3<double> p3(V(F(i, 2), 0), V(F(i, 2), 1), V(F(i, 2), 2));

        // Get two edges of this triangle p1, p2, p3
        Vector3<double> u = p2 - p1;
        Vector3<double> v = p3 - p1;

        // Calculate normal as u vector cross product v
        Vector3<double> n(u[1] * v[2] - u[2] * v[1],
                          u[2] * v[0] - u[0] * v[2],
                          u[0] * v[1] - u[1] * v[0]);

        // clang-format off
        std::cout << "F " << i
                  << " {" << p1 << ", " << p2 << ", " << p3 << "}"
                  << " n " << n
                  << std::endl;
        // clang-format on
    }
}

static void setPoint(Eigen::MatrixXd &V,
                     std::vector<double> &XY,
                     size_t idx,
                     double x,
                     double y,
                     double z)
{
    V(static_cast<Eigen::Index>(idx), 0) = x;
    V(static_cast<Eigen::Index>(idx), 1) = y;
    V(static_cast<Eigen::Index>(idx), 2) = z;

    XY[2 * idx] = x;
    XY[2 * idx + 1] = y;
}

static void exampleMeshDistance()
{
    size_t nPointsMesh = 3;

    // Create 2D and 3D point mesh coordinates
    Eigen::MatrixXd V;
    V.resize(static_cast<Eigen::Index>(nPointsMesh), 3);

    std::vector<double> XY;
    XY.resize(nPointsMesh * 2);

    setPoint(V, XY, 0, 0., 0., 0.);
    setPoint(V, XY, 1, 1., 0., 0.);
    setPoint(V, XY, 2, 0., 1., 0.);

    // Create triangle mesh
    delaunator::Delaunator delaunay(XY);

    // Convert to igl triangles
    size_t nTriangles = delaunay.triangles.size() / 3;

    Eigen::MatrixXi F;
    F.resize(static_cast<Eigen::Index>(nTriangles), 3);

    for (size_t i = 0; i < nTriangles; i++)
    {
        // Swap the order of the vertices in triangle from 0, 1, 2 to 0, 2, 1.
        // This will affect the direction of the normal to face up along z.
        F(static_cast<Eigen::Index>(i), 0) =
            static_cast<int>(delaunay.triangles[i * 3]);
        F(static_cast<Eigen::Index>(i), 1) =
            static_cast<int>(delaunay.triangles[i * 3 + 2]);
        F(static_cast<Eigen::Index>(i), 2) =
            static_cast<int>(delaunay.triangles[i * 3 + 1]);
    }

    // Print mesh
    printMesh(V, F);

    // Write output file
    igl::writeOBJ("mesh.obj", V, F);

    // Create point set
    Eigen::MatrixXd P;
    P.resize(1, 3);
    P(0, 0) = 0.;
    P(0, 1) = 0.;
    P(0, 2) = 2.5;

    // Compute distances from a set of points P to a triangle mesh (V,F)
    Eigen::MatrixXd D; // List of smallest squared distances
    Eigen::MatrixXi I; // List of indices to smallest distances
    Eigen::MatrixXd C; // 3 list of closest points

    igl::point_mesh_squared_distance(P, V, F, D, I, C);

    std::cout << "D " << D.rows() << "x" << D.cols() << " " << D << std::endl;
    std::cout << "I " << I.rows() << "x" << I.cols() << " " << I << std::endl;
    std::cout << "C " << C.rows() << "x" << C.cols() << " " << C << std::endl;
}

int main()
{
    try
    {
        exampleMeshDistance();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
