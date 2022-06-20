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

/** @file Voxels.cpp */

#include <Log.hpp>
#include <Voxels.hpp>

#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("Voxels", msg)

Voxels::Voxels() : voxelSizeInput_(0), numberOfVoxels_(0)
{
}

void Voxels::clear()
{
    spaceRegion_.clear();
    voxelSizeInput_ = 0;
    numberOfVoxels_ = 0;
    resolution_.clear();
    voxelSize_.clear();

    value_.clear();
    position_.clear();

    stack_.clear();
}

void Voxels::create(const Box<double> &spaceRegion, double voxelSize)
{
    spaceRegion_ = spaceRegion;
    voxelSizeInput_ = voxelSize;

    // Compute grid resolution and actual voxel size
    for (size_t i = 0; i < 3; i++)
    {
        resolution_[i] = static_cast<size_t>(
            round(spaceRegion_.length(i) / voxelSizeInput_));

        if (resolution_[i] < 1)
        {
            resolution_[i] = 1;
        }

        voxelSize_[i] =
            spaceRegion_.length(i) / static_cast<double>(resolution_[i]);
    }

    numberOfVoxels_ = resolution_[0] * resolution_[1] * resolution_[2];

    LOG_DEBUG_LOCAL("numberOfVoxels <" << numberOfVoxels_ << ">");
    LOG_DEBUG_LOCAL("resolution <" << resolution_ << ">");
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize_ << ">");
}
