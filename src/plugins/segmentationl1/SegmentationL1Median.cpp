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

/** @file SegmentationL1Median.cpp */

#include <SegmentationL1Median.hpp>

#define LOG_MODULE_NAME "SegmentationL1Median"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1Median::median(const Eigen::MatrixXd &V,
                                  double &x,
                                  double &y,
                                  double &z,
                                  size_t iterations,
                                  double eps)
{
    double previous = 0;
    for (size_t iteration = 0; iteration < iterations; iteration++)
    {
        LOG_DEBUG(<< "Iteration <" << (iteration + 1) << "/" << iterations
                  << ">.");
        double mx = 0;
        double my = 0;
        double mz = 0;
        double denumerator = 0;
        double current = 0;

        for (Eigen::MatrixXd::Index c = 0; c < V.cols(); c++)
        {
            double px = V(0, c);
            double py = V(1, c);
            double pz = V(2, c);
            double dx = px - x;
            double dy = py - y;
            double dz = pz - z;

            double d = std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
            LOG_DEBUG(<< "d <" << d << ">.");
            if (d < 1e-6)
            {
                continue;
            }

            mx += px / d;
            my += py / d;
            mz += pz / d;
            denumerator += 1.0 / d;
            current += d * d;
        }

        if (denumerator < 1e-6)
        {
            break;
        }

        x = mx / denumerator;
        y = my / denumerator;
        z = mz / denumerator;
        LOG_DEBUG(<< "x <" << x << "> y <" << y << "> z <" << z << ">.");

        if (iteration > 0 && std::abs(current - previous) < eps)
        {
            break;
        }

        previous = current;
    }
}
