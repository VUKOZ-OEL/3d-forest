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
    struct EXPORT_EDITOR Voxel
    {
        float x;
        float y;
        float z;
        float i;
    };

    Voxels();

    void clear();
    void create(const Box<double> &spaceRegion, double voxelSize);
    size_t size() const { return numberOfVoxels_; }
    bool next(Box<double> *cell = nullptr,
              size_t *index = nullptr,
              size_t *x = nullptr,
              size_t *y = nullptr,
              size_t *z = nullptr);

    const Voxel *data() const { return data_.data(); }

    const Voxel &at(size_t x, size_t y, size_t z) const
    {
        return data_[x + y * nx_ + z * nx_ * ny_];
    }

    Voxel &at(size_t x, size_t y, size_t z)
    {
        return data_[x + y * nx_ + z * nx_ * ny_];
    }

    const Voxel &at(size_t index) const { return data_[index]; }

    Voxel &at(size_t index) { return data_[index]; }

protected:
    Box<double> spaceRegion_;
    double voxelSizeInput_;

    size_t numberOfVoxels_;
    size_t nx_;
    size_t ny_;
    size_t nz_;
    Vector3<double> voxelSize_;
    std::vector<Voxel> data_;

    std::vector<Box<size_t>> stack_;

    void push(size_t x1, size_t y1, size_t z1, size_t x2, size_t y2, size_t z2);
    void resize(size_t n);
};

#endif /* VOXELS_HPP */
