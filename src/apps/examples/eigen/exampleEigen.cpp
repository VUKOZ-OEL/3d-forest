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

/** @file exampleEigen.cpp @brief Eigen matrix example. */

#include <Eigen/Core>

#include <Error.hpp>
#include <Log.hpp>

static void print(const Eigen::MatrixXd &V)
{
    std::cout << "matrix " << V.rows() << " x " << V.cols() << " ("
              << V.rowsCapacity() << " x " << V.colsCapacity() << ") data "
              << V.data() << std::endl;

    for (Eigen::MatrixXd::Index c = 0; c < V.cols(); c++)
    {
        std::cout << " column[" << c << "] = ";
        for (Eigen::MatrixXd::Index r = 0; r < V.rows(); r++)
        {
            std::cout << "row[" << r << "] = " << V(r, c);
            if (r + 1 < V.rows())
            {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
}

static void exampleEigenMatrixResize()
{
    Eigen::MatrixXd V;

    V.resize(3, 1);
    V(0, 0) = 1.0;
    V(1, 0) = 2.0;
    V(2, 0) = 3.0;
    print(V);

    V.resize(3, 2);
    V(0, 1) = 4.0;
    V(1, 1) = 5.0;
    V(2, 1) = 6.0;
    print(V);

    V.resize(3, 1);
    print(V);

    // Output:
    // matrix 3 x 1 (3 x 1) data 0x20332a31740
    //  column[0] = row[0] = 1, row[1] = 2, row[2] = 3
    // matrix 3 x 2 (3 x 2) data 0x20332a31740
    //  column[0] = row[0] = 1, row[1] = 2, row[2] = 3
    //  column[1] = row[0] = 4, row[1] = 5, row[2] = 6
    // matrix 3 x 1 (3 x 2) data 0x20332a31740
    //  column[0] = row[0] = 1, row[1] = 2, row[2] = 3
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
