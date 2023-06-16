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

/** @file Voxel.hpp */

#ifndef VOXEL_HPP
#define VOXEL_HPP

#include <Box.hpp>

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Voxel. */
class EXPORT_EDITOR Voxel
{
public:
    static const uint32_t STATUS_IGNORED;
    static const uint32_t STATUS_VISITED;

    static const uint32_t npos = UINT32_MAX;

    uint32_t status_;

    uint32_t x_;
    uint32_t y_;
    uint32_t z_;

    double meanX_;
    double meanY_;
    double meanZ_;
    double meanElevation_;

    double descriptor_; // <0, 1>

    uint32_t elementIndex_;

    Voxel() = default;

    Voxel(uint32_t x,
          uint32_t y,
          uint32_t z,
          double meanX,
          double meanY,
          double meanZ,
          double meanElevation = 0.0);

    void clear();
    void clearState();
};

inline std::ostream &operator<<(std::ostream &os, const Voxel &obj)
{
    // clang-format off
    return os << std::fixed
              << "((" << obj.x_ << ", " << obj.y_ << ", " << obj.z_ << "), "
              << obj.status_ << ", "
              << obj.descriptor_ << ", " << obj.elementIndex_ << ", ("
              << obj.meanX_ << ", " << obj.meanY_ << ", " << obj.meanZ_ << ", "
              << obj.meanElevation_ << "))"
              << std::defaultfloat;
    // clang-format on
}

#include <WarningsEnable.hpp>

#endif /* VOXEL_HPP */
