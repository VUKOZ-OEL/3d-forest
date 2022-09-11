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

/** @file SegmentationElement.cpp */

#include <Log.hpp>
#include <SegmentationElement.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationElement", msg)

SegmentationElement::SegmentationElement()
{
    clear();
}

void SegmentationElement::clear()
{
    elementId_ = 1U;
    voxelList_.clear();
}

void SegmentationElement::computeStart(size_t voxelIndex, const Voxels &voxels)
{
    voxelList_.clear();

    const Voxel &v = voxels.at(voxelIndex);
    if (v.status_ == 0)
    {
        queue.push({v.x_, v.y_, v.z_});
    }
}

bool SegmentationElement::compute(Voxels *voxels)
{
    while (!queue.empty())
    {
        Key k = queue.front();
        queue.pop();

        size_t index = voxels->find(k.x, k.y, k.z);
        if (index == Voxels::npos)
        {
            continue;
        }

        Voxel &v = voxels->at(index);
        if (v.status_ != 0)
        {
            continue;
        }

        v.status_ |= Voxel::STATUS_VISITED;
        voxelList_.push_back(index);

        if (k.x + 1 < voxels->sizeX())
        {
            queue.push({k.x + 1, k.y, k.z});
        }
        if (k.x > 0)
        {
            queue.push({k.x - 1, k.y, k.z});
        }
        if (k.y + 1 < voxels->sizeY())
        {
            queue.push({k.x, k.y + 1, k.z});
        }
        if (k.y > 0)
        {
            queue.push({k.x, k.y - 1, k.z});
        }
        if (k.z + 1 < voxels->sizeZ())
        {
            queue.push({k.x, k.y, k.z + 1});
        }
        if (k.z > 0)
        {
            queue.push({k.x, k.y, k.z - 1});
        }
    }

    return true;
}
