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

/** @file SegmentationMap.hpp */

#ifndef SEGMENTATION_MAP_HPP
#define SEGMENTATION_MAP_HPP

#include <Voxels.hpp>

/** Segmentation Map. */
class SegmentationMap
{
public:
    SegmentationMap();

    void clear();

    size_t size() const { return map_.size(); }
    size_t pos() const { return pos_; }
    const std::vector<double> &map() const { return map_; }

    void create(const Voxels &voxels);
    void process(const Voxels &voxels, size_t nIterations = 0);

    void toImage(int *w,
                 int *h,
                 int *components,
                 int *rowBytes,
                 std::vector<unsigned char> *image) const;

private:
    std::vector<double> map_;
    size_t pos_;
    Vector3<size_t> dim_;
};

#endif /* SEGMENTATION_MAP_HPP */
