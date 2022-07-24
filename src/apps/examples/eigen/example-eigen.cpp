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

/** @file example-voxels.cpp @brief Voxels example. */

// Ignore compiler warnings from Eigen 3rd party library.
#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) ||               \
     defined(__clang__))
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    #pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <Eigen/Core>

#include <Error.hpp>
#include <Log.hpp>

static void print(const Eigen::MatrixXd &V)
{
    std::cout << "matrix " << V.rows() << " x " << V.cols() << " ("
              << V.rowsCapacity() << " x " << V.colsCapacity() << ") data "
              << V.data() << std::endl;
}

static void exampleEigenMatrixResize()
{
    Eigen::MatrixXd V;

    V.resize(3, 4);
    print(V);
    V.resize(3, 8);
    print(V);
}

int main()
{
    try
    {
        exampleEigenMatrixResize();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
