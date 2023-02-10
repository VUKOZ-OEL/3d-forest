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
                double &meanX,
                double &meanY,
                double &meanZ,
                float &descriptor);

    bool normal(Eigen::MatrixXd &V,
                double &meanX,
                double &meanY,
                double &meanZ,
                float &descriptor);

private:
    Eigen::MatrixXd xyz;
    Eigen::Matrix3d product;
    Eigen::Matrix3d eigenVectors;
    Eigen::Matrix3d eigenVectorsT;
    Eigen::Vector3d in;
    Eigen::Vector3d out;
    Eigen::Vector3d min;
    Eigen::Vector3d max;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> E;
};

#endif /* SEGMENTATION_L1_PCA_HPP */
