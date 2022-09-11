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

#include <ExportEditor.hpp>
#include <Voxels.hpp>

/** Segmentation Element. */
class EXPORT_EDITOR SegmentationElement
{
public:
    SegmentationElement();

    void clear();

    uint32_t elementId() const { return elementId_; }
    void elementIdNext() { elementId_++; }

    const std::vector<size_t> &voxelList() const { return voxelList_; }

    void computeStart(size_t voxelIndex, const Voxels &voxels);
    bool compute(Voxels *voxels);

protected:
    uint32_t elementId_;
    std::vector<size_t> voxelList_;

    struct Key
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    std::queue<Key> queue;
};

#endif /* SEGMENTATION_ELEMENT_HPP */
