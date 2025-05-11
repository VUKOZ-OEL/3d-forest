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

/** @file ComputeTreeAttributesRandomizedHoughTransform.cpp */

// Include Std.
#include <random>

// Include 3D Forest.
#include <ComputeTreeAttributesRandomizedHoughTransform.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeTreeAttributesRandomizedHoughTransform"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ComputeTreeAttributesRandomizedHoughTransform::circleFrom3Points(double &x,
                                                                      double &y,
                                                                      double &r,
                                                                      double x1,
                                                                      double y1,
                                                                      double x2,
                                                                      double y2,
                                                                      double x3,
                                                                      double y3)
{
    double a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
    if (std::abs(a) < 1e-6)
    {
        // Points are collinear.
        r = 0;
    }

    double A1 = x1 * x1 + y1 * y1;
    double A2 = x2 * x2 + y2 * y2;
    double A3 = x3 * x3 + y3 * y3;

    x = (A1 * (y2 - y3) + A2 * (y3 - y1) + A3 * (y1 - y2)) / (2.0 * a);
    y = (A1 * (x3 - x2) + A2 * (x1 - x3) + A3 * (x2 - x1)) / (2.0 * a);
    r = std::sqrt((x1 - x) * (x1 - x) + (y1 - y) * (y1 - y));
}

void ComputeTreeAttributesRandomizedHoughTransform::compute(
    FittingCircle &circle,
    const std::vector<double> &points,
    const ComputeTreeAttributesParameters &parameters)
{
    circle.r = 0;

    size_t n = points.size() / 3;
    if (n < 3)
    {
        LOG_DEBUG(<< "Not enough points.");
        return;
    }

    std::map<std::tuple<int, int, int>, int> accumulator;
    std::mt19937 rng;
    std::uniform_int_distribution<size_t> dist(0, n - 1);

    double cx;
    double cy;
    double cr;

    // Convert coordinates from points to grid cell bin in centimeters.
    double scale = (parameters.ppm / 100.0) * parameters.dbhRhtGridCm;

    for (size_t i = 0; i < parameters.dbhRhtIterationsMax; ++i)
    {
        size_t r1 = dist(rng);
        size_t r2 = dist(rng);
        size_t r3 = dist(rng);

        if (r1 == r2 || r2 == r3 || r1 == r3)
        {
            continue;
        }

        circleFrom3Points(cx,
                          cy,
                          cr,
                          points[r1 * 3],
                          points[r1 * 3 + 1],
                          points[r2 * 3],
                          points[r2 * 3 + 1],
                          points[r3 * 3],
                          points[r3 * 3 + 1]);

        if (std::isnan(cr) || std::isinf(cr) ||
            cr < std::numeric_limits<double>::epsilon())
        {
            continue;
        }

        // Quantize for voting (e.g. 1 cm resolution).
        int qx = static_cast<int>(cx / scale);
        int qy = static_cast<int>(cy / scale);
        int qr = static_cast<int>(cr / scale);

        accumulator[{qx, qy, qr}]++;
    }

    // Find max vote.
    int maxVotes = 0;
    std::tuple<int, int, int> best;
    for (const auto &[k, v] : accumulator)
    {
        if (v > maxVotes)
        {
            maxVotes = v;
            best = k;
        }
    }

    auto [qx, qy, qr] = best;

    circle.a = qx * scale;
    circle.b = qy * scale;
    circle.r = qr * scale;

    // Calculate Z as (Z max + Z min) / 2.
    double z_min = points[2];
    double z_max = points[2];

    for (size_t i = 1; i < n; i++)
    {
        if (points[(i * 3) + 2] > z_max)
        {
            z_max = points[(i * 3) + 2];
        }

        if (points[(i * 3) + 2] < z_min)
        {
            z_min = points[(i * 3) + 2];
        }
    }

    circle.z = (z_max + z_min) / 2.0;

    LOG_DEBUG(<< "Circle a <" << circle.a << "> b <" << circle.b << "> z <"
              << circle.z << "> r <" << circle.r << ">.");
}
