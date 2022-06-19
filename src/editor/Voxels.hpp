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

#include <ExportEditor.hpp>
#include <Vector3.hpp>

/** Voxels. */
class EXPORT_EDITOR Voxels
{
public:
    Voxels();

    void clear();
    void create(double x, double y, double z, double voxelSize);

protected:
    /** Voxels Cell. */
    struct Cell
    {
        size_t x;
        size_t y;
        size_t z;
        size_t d;
    };

    double dx_;
    double dy_;
    double dz_;
    double vx_;
    double vy_;
    double vz_;
    double voxelSize_;
    size_t nx_;
    size_t ny_;
    size_t nz_;
    size_t n_;

    std::vector<float> value_;
    std::vector<float> position_;
    Vector3<size_t> dim_;

    std::vector<Cell> stack_;
};

#endif /* VOXELS_HPP */
