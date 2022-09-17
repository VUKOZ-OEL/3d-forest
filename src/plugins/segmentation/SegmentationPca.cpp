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

SegmentationPca::SegmentationPca()
{
    clear();
}

void SegmentationPca::clear()
{
    xyz.clear();

    // product
    // eigenVectors
    // eigenVectorsT
    // in;
    // out;
    // min;
    // max;
    // E;
}

bool SegmentationPca::compute(Query *query,
                              Voxels *voxels,
                              Voxel *voxel,
                              const Box<double> &cell)
{
    // The number of points inside this grid cell.
    Eigen::MatrixXd::Index nPoints = 0;

    // Select points in 'cell' into point coordinates 'xyz'.
    query->selectBox(cell);
    query->selectClassifications({LasFile::CLASS_UNASSIGNED});
    query->exec();

    while (query->next())
    {
        nPoints++;
    }

    LOG_DEBUG_LOCAL("nPoints <" << nPoints << ">");

    // Enough points for PCA?
    if (nPoints < 3)
    {
        return false;
    }

    size_t voxelId = voxels->size() + 1;

    xyz.resize(3, nPoints);
    nPoints = 0;

    query->reset();
    while (query->next())
    {
        xyz(0, nPoints) = query->x();
        xyz(1, nPoints) = query->y();
        xyz(2, nPoints) = query->z();

        query->value() = voxelId;
        query->setModified();

        nPoints++;
    }

    // Compute PCA.
    bool result;
    result = compute(xyz,
                     voxel->meanX_,
                     voxel->meanY_,
                     voxel->meanZ_,
                     voxel->intensity_);

    voxel->density_ = static_cast<float>(nPoints);

    voxels->append(*voxel);

    return result;
}

bool SegmentationPca::compute(Eigen::MatrixXd &V,
                              double &meanX,
                              double &meanY,
                              double &meanZ,
                              float &descriptor)
{
    // The number of points.
    Eigen::MatrixXd::Index nPoints = V.cols();
    LOG_DEBUG_LOCAL("V cols <" << V.cols() << "> rows <" << V.rows() << ">");
    LOG_DEBUG_LOCAL("nPoints <" << nPoints << ">");

    // Compute centroid.
    meanX = 0;
    meanY = 0;
    meanZ = 0;
    descriptor = 0;

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
    LOG_DEBUG_LOCAL("mean x <" << meanX << "> y <" << meanY << "> z <" << meanZ
                               << ">");

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
    LOG_DEBUG_LOCAL("product\n" << product);

    // Compute Eigen vectors.
    E.compute(product);
    for (Eigen::MatrixXd::Index i = 0; i < 3; i++)
    {
        eigenVectors.col(i) = E.eigenvectors().col(2 - i);
    }
    eigenVectors.col(2) = eigenVectors.col(0).cross(eigenVectors.col(1));
    LOG_DEBUG_LOCAL("eigen vectors\n" << eigenVectors);

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

    LOG_DEBUG_LOCAL("projected minimum\n" << min);
    LOG_DEBUG_LOCAL("projected maximum\n" << max);

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
    LOG_DEBUG_LOCAL("sum <" << sum << ">");
    if (sum > std::numeric_limits<double>::epsilon())
    {
        // const double SFFIx = 100. - (eL * 100. / sum);
        descriptor = static_cast<float>(eL / sum);
    }

    LOG_DEBUG_LOCAL("descriptor <" << descriptor << ">");

    return true;
}
