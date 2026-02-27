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

/** @file examplePcl.cpp @brief Pcl example. */

// Include 3rd party.
#include <pcl/point_types.h>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "examplePcl"
#include <Log.hpp>

static void examplePcl()
{
    pcl::PointXYZ p;

    p.x = 0;
    p.y = 0;
    p.z = 0;
}

int main()
{
    try
    {
        examplePcl();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
