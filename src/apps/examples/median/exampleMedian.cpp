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

/** @file exampleMedian.cpp @brief Median example. */

#include <Editor.hpp>
#include <Error.hpp>
#include <SegmentationL1Median.hpp>

#include <Log.hpp>

static void computeMedian(const Eigen::MatrixXd &points,
                          double x,
                          double y,
                          double z)
{
    SegmentationL1Median::median(points, x, y, z);

    LOG_PRINT(<< "median x <" << x << "> y <" << y << "> z <" << z << ">");
}

static void exampleMedian()
{
    //                    A    B    C
    Eigen::MatrixXd set1{{0.0, 1.0, 0.0},  // x
                         {0.0, 0.0, 1.0},  // y
                         {0.0, 0.0, 0.0}}; // z

    computeMedian(set1, 0.4, 0.0, 0.0);
}

int main()
{
    int rc = 0;
    LOGGER_START_STDOUT;
    try
    {
        exampleMedian();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        rc = 1;
    }
    catch (...)
    {
        std::cerr << "error: unknown" << std::endl;
        rc = 1;
    }
    LOGGER_STOP_STDOUT;
    return rc;
}
