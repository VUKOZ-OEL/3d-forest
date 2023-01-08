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

/** @file SegmentationMap.cpp */

#include <cstring>

#include <Log.hpp>
#include <Math.hpp>
#include <SegmentationMap.hpp>

#define MODULE_NAME "SegmentationMap"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

SegmentationMap::SegmentationMap()
{
    clear();
}

void SegmentationMap::clear()
{
    map_.clear();
    pos_ = 0;
    dim_.clear();
}

void SegmentationMap::create(const Voxels &voxels)
{
    dim_.set(voxels.sizeX(), voxels.sizeY(), voxels.sizeZ());
    map_.resize(dim_[0] * dim_[1]);
    pos_ = 0;

    std::memset(map_.data(), 0, map_.size() * sizeof(double));
}

void SegmentationMap::process(const Voxels &voxels, size_t nIterations)
{
    for (size_t z = 0; z < dim_[2]; z++)
    {
        for (size_t y = 0; y < dim_[1]; y++)
        {
            for (size_t x = 0; x < dim_[0]; x++)
            {
                size_t idx = voxels.find(x, y, z);
                if (idx != Voxels::npos)
                {
                    // const Voxel &voxel = voxels.at(idx);
                    // map_[x + (y * dim_[0])] += voxel.meanZ_;
                    // map_[x + (y * dim_[0])] += static_cast<double>(z);
                    map_[x + (y * dim_[0])] += 1.0;
                }
            }
        }
    }
}

void SegmentationMap::toImage(int *w,
                              int *h,
                              int *components,
                              int *rowBytes,
                              std::vector<unsigned char> *image) const
{
    *w = static_cast<int>(dim_[0]);
    *h = static_cast<int>(dim_[1]);
    *components = 1;
    *rowBytes = (*w) * (*components);

    size_t n = map_.size();
    image->resize(n);

    if (n < 1)
    {
        return;
    }

    double mapMin = map_[0];
    double mapMax = map_[0];

    for (size_t i = 1; i < n; i++)
    {
        updateRange(map_[i], mapMin, mapMax);
    }

    unsigned char *out = image->data();

    for (size_t i = 0; i < n; i++)
    {
        double norm = map_[i];
        normalize(norm, mapMin, mapMax);
        out[i] = static_cast<unsigned char>(norm * 255.0);
    }
}
