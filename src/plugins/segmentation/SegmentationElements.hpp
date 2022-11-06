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

/** @file SegmentationElements.hpp */

#ifndef SEGMENTATION_ELEMENTS_HPP
#define SEGMENTATION_ELEMENTS_HPP

#include <SegmentationElement.hpp>

/** Segmentation Elements. */
class SegmentationElements
{
public:
    static const uint32_t npos = UINT32_MAX;

    SegmentationElements();

    void clear();

    uint32_t computeBase(Voxels &voxels,
                         size_t voxelIndex,
                         double minimumHeight,
                         double radius);

    size_t size() const { return elements_.size(); }
    const SegmentationElement &operator[](size_t index) const
    {
        return *elements_[index];
    }

private:
    std::vector<std::shared_ptr<SegmentationElement>> elements_;
    std::shared_ptr<SegmentationElement> newElement_;
};

#endif /* SEGMENTATION_ELEMENTS_HPP */
