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

#include <cstring>

#include <Log.hpp>
#include <Math.hpp>
#include <Voxels.hpp>

//#define LOG_DEBUG_LOCAL(msg)
#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("Voxels", msg)

// Use some maximum until the voxels can be streamed from a file.
#define VOXELS_RESOLUTION_MAX 500

#if !defined(EXPORT_CORE_IMPORT)
const size_t Voxels::npos = SIZE_MAX;
#endif

Voxels::Voxels()
{
    clear();
}

void Voxels::clear()
{
    // Region
    spaceRegion_.clear();
    voxelSizeInput_ = 0;
    voxelSize_.clear();

    // Index
    nx_ = 0;
    ny_ = 0;
    nz_ = 0;
    index_.clear();

    // Voxels
    voxels_.clear();

    // Values
    intensityMin_ = std::numeric_limits<float>::max();
    intensityMax_ = std::numeric_limits<float>::min();

    densityMin_ = std::numeric_limits<float>::max();
    densityMax_ = std::numeric_limits<float>::min();

    // occupiedClear();
    // elementsClear();
    // clustersClear();

    // Create
    stack_.clear();
}

void Voxels::append(const Voxel &voxel)
{
    index_[indexOf(voxel)] = voxels_.size();
    voxels_.push_back(voxel);

    updateRange(voxel.intensity_, intensityMin_, intensityMax_);
    updateRange(voxel.density_, densityMin_, densityMax_);

    LOG_DEBUG_LOCAL("intensity <" << voxel.intensity_ << "> min <"
                                  << intensityMin_ << "> max <" << intensityMax_
                                  << ">");
}

void Voxels::normalize(Voxel *voxel)
{
    ::normalize(voxel->intensity_, intensityMin_, intensityMax_);
    ::normalize(voxel->density_, densityMin_, densityMax_);
}

void Voxels::create(const Box<double> &spaceRegion, double voxelSize)
{
    clear();

    spaceRegion_ = spaceRegion;
    voxelSizeInput_ = voxelSize;

    // Compute grid resolution and actual voxel size.
    size_t min = 1;
    size_t max = VOXELS_RESOLUTION_MAX;

    nx_ = static_cast<size_t>(round(spaceRegion_.length(0) / voxelSizeInput_));
    clamp(nx_, min, max);
    voxelSize_[0] = spaceRegion_.length(0) / static_cast<double>(nx_);

    ny_ = static_cast<size_t>(round(spaceRegion_.length(1) / voxelSizeInput_));
    clamp(ny_, min, max);
    voxelSize_[1] = spaceRegion_.length(1) / static_cast<double>(ny_);

    nz_ = static_cast<size_t>(round(spaceRegion_.length(2) / voxelSizeInput_));
    clamp(nz_, min, max);
    voxelSize_[2] = spaceRegion_.length(2) / static_cast<double>(nz_);

    // Create voxel index.
    size_t n = nx_ * ny_ * nz_;
    index_.resize(n);
    std::fill(index_.begin(), index_.end(), npos);

    // Initialize voxel iterator.
    push(0, 0, 0, nx_, ny_, nz_);

    LOG_DEBUG_LOCAL("numberOfVoxels <" << index_.size() << ">");
    LOG_DEBUG_LOCAL("resolution <" << nx_ << "," << ny_ << "," << nz_ << ">");
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize_ << ">");
}

bool Voxels::next(Voxel *voxel, Box<double> *cell)
{
    std::memset(voxel, 0, sizeof(Voxel));
    return next(cell, nullptr, &voxel->x_, &voxel->y_, &voxel->z_);
}

bool Voxels::next(Box<double> *cell,
                  size_t *index,
                  uint32_t *x,
                  uint32_t *y,
                  uint32_t *z)
{
    // Subdivide grid until next voxel cell 1x1x1 is found.
    while (!stack_.empty())
    {
        // Get next cell to process.
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

        // a) Return voxel cell 1x1x1.
        if (dx == 1 && dy == 1 && dz == 1)
        {
            if (cell)
            {
                cell->set(spaceRegion_.min(0) +
                              voxelSize_[0] * static_cast<double>(x1),
                          spaceRegion_.min(1) +
                              voxelSize_[1] * static_cast<double>(y1),
                          spaceRegion_.min(2) +
                              voxelSize_[2] * static_cast<double>(z1),
                          spaceRegion_.min(0) +
                              voxelSize_[0] * static_cast<double>(x2),
                          spaceRegion_.min(1) +
                              voxelSize_[1] * static_cast<double>(y2),
                          spaceRegion_.min(2) +
                              voxelSize_[2] * static_cast<double>(z2));
            }

            if (index)
            {
                *index = x1 + y1 * nx_ + z1 * nx_ * ny_;
            }

            if (x)
            {
                *x = static_cast<uint32_t>(x1);
            }

            if (y)
            {
                *y = static_cast<uint32_t>(y1);
            }

            if (z)
            {
                *z = static_cast<uint32_t>(z1);
            }

            return true;
        }

        // b) Subdivide cell 2x2x2, 2x1x1, etc.
        size_t px;
        size_t py;
        size_t pz;

        if (dx >= dy && dx >= dz)
        {
            px = dx / 2;
            py = dx / 2;
            pz = dx / 2;
        }
        else if (dy >= dx && dy >= dz)
        {
            px = dy / 2;
            py = dy / 2;
            pz = dy / 2;
        }
        else
        {
            px = dz / 2;
            py = dz / 2;
            pz = dz / 2;
        }

        if (x1 + px > x2)
        {
            px = dx;
        }
        if (y1 + py > y2)
        {
            py = dy;
        }
        if (z1 + pz > z2)
        {
            pz = dz;
        }

        // Push sub-cells in reverse order to iteration.
        // Creates linear order of an Octree using Morton space filling curve.
        push(x1 + px, y1 + py, z1 + pz, x2, y2, z2);
        push(x1, y1 + py, z1 + pz, x1 + px, y2, z2);
        push(x1 + px, y1, z1 + pz, x2, y1 + py, z2);
        push(x1, y1, z1 + pz, x1 + px, y1 + py, z2);

        push(x1 + px, y1 + py, z1, x2, y2, z1 + pz);
        push(x1, y1 + py, z1, x1 + px, y2, z1 + pz);
        push(x1 + px, y1, z1, x2, y1 + py, z1 + pz);
        push(x1, y1, z1, x1 + px, y1 + py, z1 + pz);
    }

    return false;
}

void Voxels::push(size_t x1,
                  size_t y1,
                  size_t z1,
                  size_t x2,
                  size_t y2,
                  size_t z2)
{
    if (x1 != x2 && y1 != y2 && z1 != z2)
    {
        stack_.push_back(Box<size_t>(x1, y1, z1, x2, y2, z2));
    }
}
