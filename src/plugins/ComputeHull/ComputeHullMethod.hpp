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

/** @file ComputeHullMethod.hpp */

#ifndef COMPUTE_HULL_METHOD_HPP
#define COMPUTE_HULL_METHOD_HPP

// Include 3D Forest.
#include <ComputeHullData.hpp>
#include <Mesh.hpp>

/** Compute Hull Method. */
class ComputeHullMethod
{
public:
    static void qhull3d(Mesh &mesh,
                        Vector3<double> &center,
                        const std::vector<double> &points);

    static void qhull2d(Mesh &mesh, const std::vector<double> &points, float z);

    static void alphaShape3(Mesh &mesh,
                            double &meshVolume,
                            double &meshSurfaceArea,
                            Vector3<double> &center,
                            const std::vector<double> &points,
                            double alpha);

    static void alphaShape2(Mesh &mesh,
                            const std::vector<double> &points,
                            double alpha,
                            float z);

    static double surface2(const Mesh &mesh);
};

#endif /* COMPUTE_HULL_METHOD_HPP */
