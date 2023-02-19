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

/** @file SegmentationL1Pca.cpp */

#include <LasFile.hpp>
#include <SegmentationL1Pca.hpp>

#define LOG_MODULE_NAME "SegmentationL1Pca"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

SegmentationL1Pca::SegmentationL1Pca()
{
    clear();
}

void SegmentationL1Pca::clear()
{
    xyz_.clear();

    // product_
    // eigenVectors_
    // eigenValues_
    // eigenSolver_;
}

bool SegmentationL1Pca::normal(Query &query,
                               double x,
                               double y,
                               double z,
                               double radius,
                               double &nx,
                               double &ny,
                               double &nz,
                               double &vx,
                               double &vy,
                               double &vz)
{
    // The number of points inside sphere[x, y, z, radius].
    Eigen::MatrixXd::Index nPoints = 0;

    // Select points in 'cell' into point coordinates 'xyz_'.
    // Count the number of points.
    query.where().setSphere(x, y, z, radius);
    query.exec();

    while (query.next())
    {
        nPoints++;
    }

    LOG_DEBUG(<< "Found nPoints <" << nPoints << ">.");

    // Enough points for PCA?
    if (nPoints < 3)
    {
        return false;
    }

    // Get point coordinates into 'xyz_'.
    xyz_.resize(3, nPoints);
    nPoints = 0;

    query.reset();
    while (query.next())
    {
        xyz_(0, nPoints) = query.x();
        xyz_(1, nPoints) = query.y();
        xyz_(2, nPoints) = query.z();

        nPoints++;
    }

    // Compute PCA.
    bool result = normal(xyz_, nx, ny, nz, vx, vy, vz);

    return result;
}

bool SegmentationL1Pca::normal(Eigen::MatrixXd &V,
                               double &nx,
                               double &ny,
                               double &nz,
                               double &vx,
                               double &vy,
                               double &vz)
{
    // The number of points.
    Eigen::MatrixXd::Index nPoints = V.cols();
    LOG_DEBUG(<< "V cols <" << V.cols() << "> rows <" << V.rows() << ">.");
    LOG_DEBUG(<< "Compute nPoints <" << nPoints << ">.");

    // Compute centroid.
    double meanX = 0;
    double meanY = 0;
    double meanZ = 0;

    for (Eigen::MatrixXd::Index i = 0; i < nPoints; i++)
    {
        meanX += V(0, i);
        meanY += V(1, i);
        meanZ += V(2, i);
    }

    if (nPoints < 3)
    {
        return false;
    }

    const double d = static_cast<double>(nPoints);
    meanX = meanX / d;
    meanY = meanY / d;
    meanZ = meanZ / d;
    LOG_DEBUG(<< "Mean x <" << meanX << "> y <" << meanY << "> z <" << meanZ
              << ">.");

    // Shift point coordinates by centroid.
    for (Eigen::MatrixXd::Index i = 0; i < nPoints; i++)
    {
        V(0, i) -= meanX;
        V(1, i) -= meanY;
        V(2, i) -= meanZ;
    }

    // Compute product.
    const double inv = 1.0 / static_cast<double>(nPoints - 1);
    product_ = inv * V.topRows<3>() * V.topRows<3>().transpose();
    LOG_DEBUG(<< "Product\n" << product_);

    // Compute Eigen vectors.
    eigenSolver_.compute(product_);

    // Pick the vector with smallest eigenvalue as normal.
    const Eigen::Matrix3d &eigenVectors = eigenSolver_.eigenvectors();
    const Eigen::Vector3d &eigenValues = eigenSolver_.eigenvalues();
    Eigen::MatrixXd::Index smallest = 0;
    Eigen::MatrixXd::Index largest = 1;
    for (Eigen::MatrixXd::Index i = 0; i < 3; i++)
    {
        if (eigenValues[i] < eigenValues[smallest])
        {
            smallest = i;
        }
        if (eigenValues[i] > eigenValues[largest])
        {
            largest = i;
        }
    }

    LOG_DEBUG(<< "Eigen values\n" << eigenValues);
    LOG_DEBUG(<< "Eigen vectors\n" << eigenVectors);

    nx = eigenVectors(0, smallest);
    ny = eigenVectors(1, smallest);
    nz = eigenVectors(2, smallest);

    vx = eigenVectors(0, largest);
    vy = eigenVectors(1, largest);
    vz = eigenVectors(2, largest);

    return true;
}
