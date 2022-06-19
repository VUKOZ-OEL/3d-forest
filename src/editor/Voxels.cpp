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

#include <Voxels.hpp>

Voxels::Voxels()
{
}

void Voxels::clear()
{
    value_.clear();
}

void Voxels::create(double x, double y, double z, double voxelSize)
{
    dx_ = x;
    dy_ = y;
    dz_ = z;
    voxelSize_ = voxelSize;

    nx_ = static_cast<size_t>(round(dx_ / voxelSize_));
    ny_ = static_cast<size_t>(round(dy_ / voxelSize_));
    nz_ = static_cast<size_t>(round(dz_ / voxelSize_));
    if (nx_ < 1)
    {
        nx_ = 1;
    }
    if (ny_ < 1)
    {
        ny_ = 1;
    }
    if (nz_ < 1)
    {
        nz_ = 1;
    }
    vx_ = dx_ / static_cast<double>(nx_);
    vy_ = dy_ / static_cast<double>(ny_);
    vz_ = dz_ / static_cast<double>(nz_);
    n_ = nx_ * ny_ * nz_;
}
