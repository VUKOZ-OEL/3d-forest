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

    stack_.push_back(Box<size_t>(static_cast<size_t>(0),
                                 static_cast<size_t>(0),
                                 static_cast<size_t>(0),
                                 resolution_[0],
                                 resolution_[1],
                                 resolution_[2]));

    LOG_DEBUG_LOCAL("numberOfVoxels <" << numberOfVoxels_ << ">");
    LOG_DEBUG_LOCAL("resolution <" << resolution_ << ">");
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize_ << ">");
}

bool Voxels::next(Box<double> &cell)
{
    while (!stack_.empty())
    {
        const Box<size_t> &c = stack_.back();
        size_t x1 = c.min(0);
        size_t y1 = c.min(1);
        size_t z1 = c.min(2);
        size_t x2 = c.max(0);
        size_t y2 = c.max(1);
        size_t z2 = c.max(2);
        size_t dx = x2 - x1;
        size_t dy = y2 - y1;
        size_t dz = z2 - z1;
        stack_.pop_back();

        if (dx < 1 || dy < 1 || dz < 1)
        {
            continue;
        }

        if (dx == 1 && dy == 1 && dz == 1)
        {
            cell.set(
                spaceRegion_.min(0) + voxelSize_[0] * static_cast<double>(x1),
                spaceRegion_.min(1) + voxelSize_[1] * static_cast<double>(y1),
                spaceRegion_.min(2) + voxelSize_[2] * static_cast<double>(z1),
                spaceRegion_.min(0) + voxelSize_[0] * static_cast<double>(x2),
                spaceRegion_.min(1) + voxelSize_[1] * static_cast<double>(y2),
                spaceRegion_.min(2) + voxelSize_[2] * static_cast<double>(z2));

            return true;
        }

        size_t px = dx / 2;
        size_t py = dy / 2;
        size_t pz = dz / 2;

        // Push in reverse order to iteration
        stack_.push_back(Box<size_t>(x1 + px, y1 + py, z1 + pz, x2, y2, z2));
        stack_.push_back(Box<size_t>(x1, y1 + py, z1 + pz, x1 + px, y2, z2));
        stack_.push_back(Box<size_t>(x1 + px, y1, z1 + pz, x2, y1 + py, z2));
        stack_.push_back(Box<size_t>(x1, y1, z1 + pz, x1 + px, y1 + py, z2));

        stack_.push_back(Box<size_t>(x1 + px, y1 + py, z1, x2, y2, z1 + pz));
        stack_.push_back(Box<size_t>(x1, y1 + py, z1, x1 + px, y2, z1 + pz));
        stack_.push_back(Box<size_t>(x1 + px, y1, z1, x2, y1 + py, z1 + pz));
        stack_.push_back(Box<size_t>(x1, y1, z1, x1 + px, y1 + py, z1 + pz));
    }

    return false;
}
