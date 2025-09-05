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

/** @file ComputeSkeletonMethod.hpp */

#ifndef COMPUTE_SKELETON_METHOD_HPP
#define COMPUTE_SKELETON_METHOD_HPP

// Include 3D Forest.
#include <ComputeSkeletonData.hpp>
#include <ComputeSkeletonParameters.hpp>

/** Compute Skeleton Method. */
class ComputeSkeletonMethod
{
public:
    static void compute(ComputeSkeletonData &data,
                        const ComputeSkeletonParameters &parameters);
};

#endif /* COMPUTE_SKELETON_METHOD_HPP */
