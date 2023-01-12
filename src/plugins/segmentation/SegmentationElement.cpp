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

#define LOG_MODULE_NAME "SegmentationElement"

SegmentationElement::SegmentationElement()
{
    clear();
}

void SegmentationElement::clear()
{
    start_.clear();
    base_.clear();

    radius_ = 0.0;
    height_ = 0.0;

    std::queue<Key> queueEmpty_;
    std::swap(queue_, queueEmpty_);

    voxelList_.clear();

    elementIndex_ = 0;
}

bool SegmentationElement::computeStart(const Voxels &voxels,
                                       size_t voxelIndex,
                                       double radius)
{
    clear();

    const Voxel &v = voxels.sortedAt(voxelIndex);

    if (v.status_ == 0)
    {
        queue_.push({v.x_, v.y_, v.z_});
        start_.set(v.meanX_, v.meanY_, v.meanZ_);
        base_ = start_;
        radius_ = radius;
        return true;
    }

    return false;
}

bool SegmentationElement::computeBase(Voxels &voxels, double minimumHeight)
{
    while (!queue_.empty())
    {
        // get current voxel
        Key k = queue_.front();
        queue_.pop();

        size_t index = voxels.find(k.x, k.y, k.z);
        if (index == Voxels::npos)
        {
            continue;
        }

        Voxel &v = voxels.at(index);
        if (v.status_ != 0)
        {
            continue;
        }

        // mark as processed
        v.status_ |= Voxel::STATUS_VISITED;
        voxelList_.push_back(index);

        // check height
        height_ = v.meanZ_ - base_[2];
        if (!(height_ < minimumHeight))
        {
            return true;
        }

        pushNext1(k, voxels);
    }

    return false;
}

void SegmentationElement::pushNext1(const Key &k, const Voxels &voxels)
{
    if (k.z + 1 < voxels.sizeZ())
    {
        if (k.x > 0 && k.x + 1 < voxels.sizeX() && k.y > 0 &&
            k.y + 1 < voxels.sizeY())
        {
            queue_.push({k.x, k.y, k.z + 1});

            queue_.push({k.x + 1, k.y, k.z + 1});
            queue_.push({k.x - 1, k.y, k.z + 1});
            queue_.push({k.x, k.y + 1, k.z + 1});
            queue_.push({k.x, k.y - 1, k.z + 1});

            queue_.push({k.x + 1, k.y + 1, k.z + 1});
            queue_.push({k.x + 1, k.y - 1, k.z + 1});
            queue_.push({k.x - 1, k.y + 1, k.z + 1});
            queue_.push({k.x - 1, k.y - 1, k.z + 1});
        }
    }
}
