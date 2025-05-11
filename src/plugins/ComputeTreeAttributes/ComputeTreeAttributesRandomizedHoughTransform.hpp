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

/** @file ComputeTreeAttributesRandomizedHoughTransform.hpp */

#ifndef COMPUTE_TREE_ATTRIBUTES_RANDOMIZED_HOUGH_TRANSFORM_HPP
#define COMPUTE_TREE_ATTRIBUTES_RANDOMIZED_HOUGH_TRANSFORM_HPP

// Include 3D Forest.
#include <ComputeTreeAttributesData.hpp>
#include <ComputeTreeAttributesParameters.hpp>

/** Compute Tree Attributes Randomized Hough Transform. */
class ComputeTreeAttributesRandomizedHoughTransform
{
public:
    /** Compute Tree Attributes Randomized Hough Transform Fitting Circle. */
    class FittingCircle
    {
    public:
        double a{0.0}; /**< X-coordinate of the center of the fitting circle. */
        double b{0.0}; /**< Y-coordinate of the center of the fitting circle. */
        double z{0.0}; /**< Z-coordinate of the center of the fitting circle. */
        double r{0.0}; /**< Radius of the fitting circle. */
    };

    static void circleFrom3Points(double &x,
                                  double &y,
                                  double &r,
                                  double x1,
                                  double y1,
                                  double x2,
                                  double y2,
                                  double x3,
                                  double y3);

    static void compute(FittingCircle &circle,
                        const std::vector<double> &points,
                        const ComputeTreeAttributesParameters &parameters);
};

#endif /* COMPUTE_TREE_ATTRIBUTES_RANDOMIZED_HOUGH_TRANSFORM_HPP */
