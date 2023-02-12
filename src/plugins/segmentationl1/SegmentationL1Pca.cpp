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
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

SegmentationL1Pca::SegmentationL1Pca()
{
    clear();
}

void SegmentationL1Pca::clear()
{
    xyz.clear();

    // product
    // eigenVectors
    // eigenVectorsT
    // eigenValues
    // in;
    // out;
    // min;
    // max;
    // E;
}

bool SegmentationL1Pca::normal(Query &query,
                               double x,
                               double y,
                               double z,
                               double radius,
                               double &nx,
                               double &ny,
                               double &nz)
{
    // The number of points inside sphere[x, y, z, radius].
    Eigen::MatrixXd::Index nPoints = 0;

    // Select points in 'cell' into point coordinates 'xyz'.
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

    // Get point coordinates into 'xyz'.
    xyz.resize(3, nPoints);
    nPoints = 0;

    query.reset();
    while (query.next())
    {
        xyz(0, nPoints) = query.x();
        xyz(1, nPoints) = query.y();
        xyz(2, nPoints) = query.z();

        nPoints++;
    }

    // Compute PCA.
    bool result = normal(xyz, nx, ny, nz);

    return result;
}

bool SegmentationL1Pca::normal(Eigen::MatrixXd &V,
                               double &nx,
                               double &ny,
                               double &nz)
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
    product = inv * V.topRows<3>() * V.topRows<3>().transpose();
    LOG_DEBUG(<< "Product\n" << product);

    // Compute Eigen vectors.
    E.compute(product);

    // Reorder.
    Eigen::MatrixXd::Index smallest = 0;
    eigenValues[0] = E.eigenvalues()[2];
    eigenVectors.col(0) = E.eigenvectors().col(2);
    for (Eigen::MatrixXd::Index i = 1; i < 3; i++)
    {
        eigenValues[i] = E.eigenvalues()[2 - i];
        eigenVectors.col(i) = E.eigenvectors().col(2 - i);

        if (eigenValues[i] < eigenValues[smallest])
        {
            smallest = i;
        }
    }
    eigenVectors.col(2) = eigenVectors.col(0).cross(eigenVectors.col(1));
    LOG_DEBUG(<< "Eigen values\n" << eigenValues);
    LOG_DEBUG(<< "Eigen vectors\n" << eigenVectors);

    nx = eigenVectors(0, smallest);
    ny = eigenVectors(1, smallest);
    nz = eigenVectors(2, smallest);

#if 0
    // Project point coordinates by Eigen vectors.
    constexpr double bigNumber = std::numeric_limits<double>::max();
    min[0] = bigNumber;
    min[1] = bigNumber;
    min[2] = bigNumber;
    max[0] = -bigNumber;
    max[1] = -bigNumber;
    max[2] = -bigNumber;
    eigenVectorsT = eigenVectors.transpose();
    for (Eigen::MatrixXd::Index i = 0; i < nPoints; i++)
    {
        in[0] = V(0, i);
        in[1] = V(1, i);
        in[2] = V(2, i);

        out = eigenVectorsT * in;

        min = min.cwiseMin(out);
        max = max.cwiseMax(out);
    }

    LOG_DEBUG(<< "Projected minimum\n" << min);
    LOG_DEBUG(<< "Projected maximum\n" << max);

    // Compute intensity.
    double eL = std::abs(max[0] - min[0]);
    double eI = std::abs(max[1] - min[1]);
    double eS = std::abs(max[2] - min[2]);

    LOG_DEBUG(<< "Computed eLIS <" << eL << "," << eI << "," << eS << ">.");
#endif
    return true;
}
