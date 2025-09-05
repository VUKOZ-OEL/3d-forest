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

/** @file ComputeSkeletonMethod.cpp */

// Include Std.
#include <random>

// Include 3D Forest.
#include <ComputeSkeletonMethod.hpp>
#include <Util.hpp>

// Include 3rd party.
#include <skeleton.h>

// Include local.
#define LOG_MODULE_NAME "ComputeSkeletonMethod"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void ComputeSkeletonMethod::compute(ComputeSkeletonData &data,
                                    const ComputeSkeletonParameters &parameters)
{
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud;
    f3d::Skeleton s(cloud, 0.5, 1);
}
