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

/** @file SegmentationL1Point.hpp */

#ifndef SEGMENTATION_L1_POINT_HPP
#define SEGMENTATION_L1_POINT_HPP

#include <cstdint>
#include <sstream>

/** Segmentation L1 Point. */
class SegmentationL1Point
{
public:
    uint64_t index;
    double x;
    double y;
    double z;
    double nx;
    double ny;
    double nz;
};

inline std::ostream &operator<<(std::ostream &os,
                                const SegmentationL1Point &obj)
{
    // clang-format off
    return os << "index <" << obj.index
              << "> x <" << obj.x << "> y <" << obj.y << "> z <" << obj.z
              << "> nx <" << obj.nx << "> ny <" << obj.ny << "> nz <" << obj.nz
              << ">";
    // clang-format on
}

#endif /* SEGMENTATION_L1_POINT_HPP */
