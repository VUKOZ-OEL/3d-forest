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

/** @file SegmentationPca.cpp */

#include <LasFile.hpp>
#include <Log.hpp>
#include <SegmentationPca.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationPca", msg)

#define SEGMENTATION_PCA_POINT_BUFFER_SIZE 2000

SegmentationPca::SegmentationPca()
{
    clear();
}

void SegmentationPca::clear()
{
    intensityMin_ = std::numeric_limits<float>::max();
    intensityMax_ = std::numeric_limits<float>::min();
}

void SegmentationPca::compute(Query &query,
                              Voxels &voxels,
                              const Box<double> &cell,
                              size_t index)
{
    // Get point coordinates in voxel given by 'cell' and compute their
    // centroid. Initialize centroid.
    double meanX = 0;
    double meanY = 0;
    double meanZ = 0;

    // Initialize point coordinate buffer and reserve memory.
    Eigen::MatrixXd::Index nPoints = 0;
    V.resize(3, 0);
    if (V.colsCapacity() == 0)
    {
        V.reserve(3, SEGMENTATION_PCA_POINT_BUFFER_SIZE);
    }

    // Select points in 'cell' into point coordinates and centroid.
    query.selectBox(cell);
    query.selectClassifications({LasFile::CLASS_UNASSIGNED});
    query.exec();

    while (query.next())
    {
        if (nPoints == V.colsCapacity())
        {
            V.reserve(3, nPoints * 2);
        }

        V.resize(3, nPoints + 1);

        meanX += query.x();
        V(0, nPoints) = query.x();

        meanY += query.y();
        V(1, nPoints) = query.y();

        meanZ += query.z();
        V(2, nPoints) = query.z();

        nPoints++;
    }

    // Create voxel.
    if (nPoints > 0)
    {
        const double d = static_cast<double>(nPoints);
        meanX = meanX / d;
        meanY = meanY / d;
        meanZ = meanZ / d;
    }
    else
    {
        cell.getCenter(meanX, meanY, meanZ);
    }

    LOG_DEBUG_LOCAL("nPoints <" << nPoints << ">");
    LOG_DEBUG_LOCAL("mean <" << meanX << "," << meanY << "," << meanZ << ">");

    Voxels::Voxel &voxel = voxels.at(index);
    voxel.x = static_cast<float>(meanX);
    voxel.y = static_cast<float>(meanY);
    voxel.z = static_cast<float>(meanZ);
    voxel.i = 0;

    // Enough points for PCA?
    if (nPoints < 3)
    {
        return;
    }

    // Shift point coordinates by centroid.
    LOG_DEBUG_LOCAL("V cols <" << V.cols() << "> rows <" << V.rows() << ">");
    for (Eigen::MatrixXd::Index i = 0; i < nPoints; i++)
    {
        V(0, i) -= meanX;
        V(1, i) -= meanY;
        V(2, i) -= meanZ;
    }

    // Compute product.
    const double inv = 1. / static_cast<double>(nPoints - 1);
    product = inv * V.topRows<3>() * V.topRows<3>().transpose();
    LOG_DEBUG_LOCAL("product\n" << product);

    // Compute Eigen vectors.
    E.compute(product);
    for (size_t i = 0; i < 3; i++)
    {
        eigenVectors.col(i) = E.eigenvectors().col(2 - i);
    }
    eigenVectors.col(2) = eigenVectors.col(0).cross(eigenVectors.col(1));
    LOG_DEBUG_LOCAL("eigen vectors\n" << eigenVectors);

    // Project point coordinates by eigen vectors.
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

    LOG_DEBUG_LOCAL("min\n" << min);
    LOG_DEBUG_LOCAL("max\n" << max);

    // Compute intensity.
    double eL = std::abs(max[0] - min[0]);
    double eI = std::abs(max[1] - min[1]);
    double eS = std::abs(max[2] - min[2]);

    // Sort values.
    LOG_DEBUG_LOCAL("eLIS <" << eL << "," << eI << "," << eS << ">");
    if (eI < eS)
    {
        std::swap(eS, eI);
    }

    if (eL < eI)
    {
        std::swap(eL, eI);
    }

    if (eI < eS)
    {
        std::swap(eS, eI);
    }
    LOG_DEBUG_LOCAL("eLIS <" << eL << "," << eI << "," << eS << "> sorted");

    // Compute intensity index.
    const double sum = eL + eI + eS;
    if (sum > std::numeric_limits<double>::epsilon())
    {
        const double SFFIx = 100. - (eL * 100. / sum);
        voxel.i = static_cast<float>(SFFIx);

        if (voxel.i < intensityMin_)
        {
            intensityMin_ = voxel.i;
        }

        if (voxel.i > intensityMax_)
        {
            intensityMax_ = voxel.i;
        }

        LOG_DEBUG_LOCAL("intensity <" << voxel.i << "> min <" << intensityMin_
                                      << "> max <" << intensityMax_ << ">");
    }
}
