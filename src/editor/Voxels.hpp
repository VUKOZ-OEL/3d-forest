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

/** @file Voxels.hpp */

#ifndef VOXELS_HPP
#define VOXELS_HPP

#include <Box.hpp>
#include <ExportEditor.hpp>

/** Voxels. */
class EXPORT_EDITOR Voxels
{
public:
    Voxels();

    void clear();
    void create(const Box<double> &spaceRegion, double voxelSize);
    size_t size() const { return numberOfVoxels_; }
    bool next(Box<double> &cell);

protected:
    Box<double> spaceRegion_;
    double voxelSizeInput_;

    size_t numberOfVoxels_;
    Vector3<size_t> resolution_;
    Vector3<double> voxelSize_;

    std::vector<float> value_;
    std::vector<float> position_;

    std::vector<Box<size_t>> stack_;
};

#endif /* VOXELS_HPP */
