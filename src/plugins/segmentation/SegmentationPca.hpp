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

/** @file SegmentationPca.hpp */

#ifndef SEGMENTATION_PCA_HPP
#define SEGMENTATION_PCA_HPP

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <Query.hpp>
#include <Voxels.hpp>

/** Segmentation PCA. */
class SegmentationPca
{
public:
    SegmentationPca();

    void clear();

    bool compute(Query *query, Voxel *voxel, const Box<double> &cell);

protected:
    Eigen::MatrixXd V;
    Eigen::Matrix3d product;
    Eigen::Matrix3d eigenVectors;
    Eigen::Matrix3d eigenVectorsT;
    Eigen::Vector3d in;
    Eigen::Vector3d out;
    Eigen::Vector3d min;
    Eigen::Vector3d max;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> E;
};

#endif /* SEGMENTATION_PCA_HPP */
