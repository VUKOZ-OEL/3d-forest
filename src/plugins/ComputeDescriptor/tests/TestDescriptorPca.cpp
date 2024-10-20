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

/** @file TestComputeDescriptorPca.cpp */

#include <ComputeDescriptorPca.hpp>
#include <Test.hpp>
#include <Util.hpp>

#define LOG_MODULE_NAME "TestComputeDescriptorPca"
#include <Log.hpp>

static double testComputeDescriptorPca(const Eigen::MatrixXd &points)
{
    Eigen::MatrixXd V(points);
    double x = 0;
    double y = 0;
    double z = 0;
    double descriptor = 0;

    ComputeDescriptorPca pca;
    (void)pca.computeDescriptor(V, x, y, z, descriptor);

    return descriptor;
}

TEST_CASE(TestComputeDescriptorPcaCube)
{
    // Cube,      points:   A    B    C    D    E    F    G    H
    Eigen::MatrixXd cube{{0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0},  // x
                         {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0},  // y
                         {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0}}; // z

    TEST(between(testComputeDescriptorPca(cube), 0.32, 0.34)); // 0.33
}

TEST_CASE(TestComputeDescriptorPcaPlane)
{
    // Plane,      points:   A    B    C    D    E    F    G    H
    Eigen::MatrixXd plane{{0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0},  // x
                          {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0},  // y
                          {0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1}}; // z

    TEST(between(testComputeDescriptorPca(plane), 0.47, 0.49)); // 0.48
}

TEST_CASE(TestComputeDescriptorPcaLine)
{
    // Line,      points:   A    B    C    D    E    F    G    H
    Eigen::MatrixXd line{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // x
                         {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // y
                         {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0}}; // z

    TEST(between(testComputeDescriptorPca(line), 0.99, 1.01)); // 1.0
}
