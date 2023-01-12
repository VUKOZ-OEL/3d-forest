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

/** @file SegmentationElements.cpp */

#include <Log.hpp>
#include <SegmentationElements.hpp>

#define LOG_MODULE_NAME "SegmentationElements"

SegmentationElements::SegmentationElements()
{
    clear();
}

void SegmentationElements::clear()
{
    elements_.clear();
}

uint32_t SegmentationElements::computeBase(Voxels &voxels,
                                           size_t voxelIndex,
                                           double minimumHeight,
                                           double radius)
{
    const Voxel &v = voxels.sortedAt(voxelIndex);

    if (v.status_ == 0)
    {
        newElement_ = std::make_shared<SegmentationElement>();
        (void)newElement_->computeStart(voxels, voxelIndex, radius);

        bool isHigh = newElement_->computeBase(voxels, minimumHeight);

        if (isHigh && elements_.size() < SegmentationElements::npos)
        {
            uint32_t elementIndex = static_cast<uint32_t>(elements_.size());
            newElement_->setElementIndex(elementIndex);
            elements_.push_back(newElement_);

            return elementIndex;
        }
    }

    return SegmentationElements::npos;
}
