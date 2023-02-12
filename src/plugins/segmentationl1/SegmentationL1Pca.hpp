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

/** @file SegmentationL1Pca.hpp */

#ifndef SEGMENTATION_L1_PCA_HPP
#define SEGMENTATION_L1_PCA_HPP

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <Query.hpp>

/** Segmentation L1 PCA. */
class SegmentationL1Pca
{
public:
    SegmentationL1Pca();

    void clear();

    bool normal(Query &query,
                double x,
                double y,
                double z,
                double radius,
                double &nx,
                double &ny,
                double &nz);

    bool normal(Eigen::MatrixXd &V, double &nx, double &ny, double &nz);

private:
    Eigen::MatrixXd xyz_;
    Eigen::Matrix3d product_;
    Eigen::Matrix3d eigenVectors_;
    Eigen::Vector3d eigenValues_;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigenSolver_;
};

#endif /* SEGMENTATION_L1_PCA_HPP */
