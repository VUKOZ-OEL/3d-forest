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

#include <Log.hpp>
#include <SegmentationPca.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationPca", msg)

SegmentationPca::SegmentationPca()
{
    clear();
}

void SegmentationPca::clear()
{
}

void SegmentationPca::compute(Query &query,
                              Voxels &voxels,
                              const Box<double> &cell,
                              size_t index)
{
    // Compute voxel centroid.
    size_t nPoints = 0;
    double meanX = 0;
    double meanY = 0;
    double meanZ = 0;

    query.selectBox(cell);
    query.exec();

    while (query.next())
    {
        meanX += query.x();
        meanY += query.y();
        meanZ += query.z();
        nPoints++;
    }

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

    // Get vertices.
    V.resize(nPoints, 3);

    query.reset();
    nPoints = 0;

    while (query.next())
    {
        V(nPoints, 0) = query.x();
        V(nPoints, 1) = query.y();
        V(nPoints, 2) = query.z();
        nPoints++;
    }

    // Shift by centroid.
    for (size_t i = 0; i < nPoints; i++)
    {
        V(i, 0) -= meanX;
        V(i, 1) -= meanY;
        V(i, 2) -= meanZ;
    }

    // Compute product.
    const double inv = 1. / static_cast<double>(nPoints);
    product = inv * V.topRows<3>() * V.topRows<3>().transpose();

    // Compute Eigen vectors.
    E.compute(product);
    for (size_t i = 0; i < 3; i++)
    {
        eigenVectors.col(i) = E.eigenvectors().col(2 - i);
    }
    eigenVectors.col(2) = eigenVectors.col(0).cross(eigenVectors.col(1));

    LOG_DEBUG_LOCAL("eigen vectors\n" << eigenVectors);

    // Project.
    constexpr double bigNumber = std::numeric_limits<double>::max();
    min[0] = bigNumber;
    min[1] = bigNumber;
    min[2] = bigNumber;
    max[0] = -bigNumber;
    max[1] = -bigNumber;
    max[2] = -bigNumber;
    eigenVectorsT = eigenVectors.transpose();
    for (size_t i = 0; i < nPoints; i++)
    {
        in[0] = V(i, 0);
        in[1] = V(i, 1);
        in[2] = V(i, 2);

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
    }
}
