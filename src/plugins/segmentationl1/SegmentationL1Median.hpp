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

/** @file SegmentationL1Median.hpp */

#ifndef SEGMENTATION_L1_MEDIAN_HPP
#define SEGMENTATION_L1_MEDIAN_HPP

#include <Query.hpp>

#include <Eigen/Core>

/** Segmentation L1 Median. */
class SegmentationL1Median
{
public:
    void clear();

    void median(Query &query,
                double &x,
                double &y,
                double &z,
                size_t iterations = 100,
                double eps = 1e-6);

    static void median(const Eigen::MatrixXd &V,
                       double &x,
                       double &y,
                       double &z,
                       size_t iterations = 100,
                       double eps = 1e-6);

    static void mean(const Eigen::MatrixXd &V, double &x, double &y, double &z);

private:
    Eigen::MatrixXd xyz_;
};

#endif /* SEGMENTATION_L1_MEDIAN_HPP */
