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

/** @file SegmentationElement.hpp */

#ifndef SEGMENTATION_ELEMENT_HPP
#define SEGMENTATION_ELEMENT_HPP

#include <queue>

#include <Vector3.hpp>
#include <Voxels.hpp>

/** Segmentation Element. */
class SegmentationElement
{
public:
    SegmentationElement();

    void clear();

    bool computeStart(const Voxels &voxels, size_t voxelIndex, double radius);
    bool computeBase(Voxels &voxels, double minimumHeight);

    const std::vector<size_t> &voxelList() const { return voxelList_; }

    void setElementIndex(uint32_t index) { elementIndex_ = index; }
    uint32_t elementIndex() const { return elementIndex_; }

private:
    Vector3<double> start_;
    Vector3<double> base_;
    double radius_;
    double height_;

    struct Key
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
    std::queue<Key> queue_;

    std::vector<size_t> voxelList_;

    uint32_t elementIndex_;

    void pushNext1(const Key &k, const Voxels &voxels);
};

#endif /* SEGMENTATION_ELEMENT_HPP */
