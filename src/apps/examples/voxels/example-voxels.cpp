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

/** @file example-voxels.cpp @brief Voxels example. */

#include <Box.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <Time.hpp>
#include <Voxels.hpp>

void exampleVoxels()
{
    Box<double> spaceRegion(0., 0., 0., 4., 2., 2.);
    double voxelSize = 1.;

    Voxels voxels;

    voxels.create(spaceRegion, voxelSize);
    std::cout << "number of voxels is " << voxels.size() << std::endl;

    Box<double> cell;
    size_t x;
    size_t y;
    size_t z;
    size_t index;

    while (voxels.next(&cell, &index, &x, &y, &z))
    {
        std::cout << index + 1 << "/" << voxels.size() << " [" << x << ", " << y
                  << ", " << z << "] " << cell << std::endl;

        Voxels::Voxel &voxel = voxels.at(x, y, z);
        voxel.x = 0;
        voxel.y = 0;
        voxel.z = 0;
        voxel.i = 0;
        voxel.state = 0;
    }
}

int main()
{
    try
    {
        exampleVoxels();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
