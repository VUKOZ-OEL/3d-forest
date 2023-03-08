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

#include <ArgumentParser.hpp>
#include <Editor.hpp>
#include <Error.hpp>
#include <SegmentationL1Median.hpp>

#include <Log.hpp>

static void computeMedian(const Eigen::MatrixXd &points,
                          size_t iterations,
                          double eps)
{
    double x;
    double y;
    double z;

    SegmentationL1Median::mean(points, x, y, z);
    LOG_PRINT(<< "mean x <" << x << "> y <" << y << "> z <" << z << ">");

    SegmentationL1Median::median(points, x, y, z, iterations, eps);
    LOG_PRINT(<< "median x <" << x << "> y <" << y << "> z <" << z << ">");
}

static void exampleMedian(size_t iterations, double eps)
{
    //                    A    B    C    D
    Eigen::MatrixXd set1{{1.0, 7.0, 9.0, 4.0},  // x
                         {3.0, 1.0, 3.0, 5.0},  // y
                         {5.0, 2.0, 1.0, 6.0}}; // z

    computeMedian(set1, iterations, eps);

    // Output:
    //   mean   x <5.25>    y <3>       z <3.5>
    //   median x <5.39402> y <2.90189> z <3.47224> -n 10 -e 0.1
    //   median x <5.59192> y <2.785>   z <3.30639> -n 10 -e 0.01
    //   median x <5.65338> y <2.74789> z <3.25515> -n 25 -e 0.000001
    //   median x <5.65837> y <2.74486> z <3.251>   -n 70 -e 0.000001 reference
}

int main(int argc, char *argv[])
{
    int rc = 0;
    LOGGER_START_STDOUT;
    try
    {
        ArgumentParser arg;
        arg.add("-n", "100");
        arg.add("-e", "0.000001");
        arg.parse(argc, argv);

        exampleMedian(arg.toSize("-n"), arg.toDouble("-e"));
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
