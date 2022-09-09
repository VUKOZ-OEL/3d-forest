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

/** @file exampleVoxels.cpp @brief Voxels example. */

#include <Box.hpp>
#include <Error.hpp>
#include <Log.hpp>
#include <Time.hpp>
#include <Voxels.hpp>

static void exampleVoxels()
{
    Box<double> spaceRegion(0., 0., 0., 4., 2., 2.);
    double voxelSize = 1.;

    Voxels voxels;

    voxels.create(spaceRegion, voxelSize);
    std::cout << "number of voxels is " << voxels.indexSize() << std::endl;

    Voxels::Voxel voxel;
    Box<double> cell;

    while (voxels.next(&voxel, &cell))
    {
        std::cout << (voxels.indexOf(voxel) + 1) << "/" << voxels.indexSize()
                  << ", " << voxel << ", " << cell << std::endl;
    }

    std::cout << "voxel occupancy <" << voxels.size() << ">" << std::endl;
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
