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

/** @file DescriptorPca.hpp */

#ifndef DESCRIPTOR_PCA_HPP
#define DESCRIPTOR_PCA_HPP

// Include 3rd party.
#include <Eigen/Core>
#include <Eigen/Eigenvalues>

// Include 3D Forest.
#include <Query.hpp>

/** Descriptor PCA. */
class DescriptorPca
{
public:
    DescriptorPca();

    void clear();

    bool computeDescriptor(Query &query,
                           const Box<double> &cell,
                           double &meanX,
                           double &meanY,
                           double &meanZ,
                           double &descriptor);

    bool computeDescriptor(Query &query,
                           double x,
                           double y,
                           double z,
                           double radius,
                           double &meanX,
                           double &meanY,
                           double &meanZ,
                           double &descriptor);

    bool computeDescriptor(Eigen::MatrixXd &V,
                           double &meanX,
                           double &meanY,
                           double &meanZ,
                           double &descriptor);

    bool computeDistribution(Query &query,
                             double x,
                             double y,
                             double z,
                             double radius,
                             double &descriptor);

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

#endif /* DESCRIPTOR_PCA_HPP */
