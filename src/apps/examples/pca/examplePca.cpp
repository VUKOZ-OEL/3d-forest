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

/** @file examplePca.cpp @brief PCA example. */

#include <Editor.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <SegmentationPca.hpp>

static void computePca(const Eigen::MatrixXd &points)
{
    Eigen::MatrixXd V(points);
    double x;
    double y;
    double z;
    float descriptor;

    SegmentationPca pca;
    (void)pca.compute(V, x, y, z, descriptor);

    std::cout << "descriptor <" << descriptor << ">"
              << " mean x <" << x << "> y <" << y << "> z <" << z << ">"
              << std::endl;
}

static void examplePca()
{
    // Cube 0.33
    Eigen::MatrixXd cube{{0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0},
                         {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0},
                         {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0}};

    computePca(cube);

    // Plane 0.48
    Eigen::MatrixXd plane{{0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0},
                          {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0},
                          {0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1}};

    computePca(plane);

    // Line 1.00
    Eigen::MatrixXd line{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                         {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                         {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0}};

    computePca(line);
}

int main()
{
    try
    {
        examplePca();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}