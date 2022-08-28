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

/** @file Elevation.hpp */

#ifndef ELEVATION_HPP
#define ELEVATION_HPP

#include <Eigen/Core>

#include <Query.hpp>

class Editor;

/** Elevation. */
class Elevation
{
public:
    Elevation(Editor *editor);
    ~Elevation();

    int start(size_t pointsPerCell = 10000, double cellLengthMinPercent = 1.);
    void step();
    void exportGroundMesh(const char *path);
    void clear();

protected:
    Editor *editor_;
    Query query_;

    int currentStep_;
    int numberOfSteps_;

    uint64_t elevationPointsCount_;
    double elevationMinimum_;
    double elevationMaximum_;

    Eigen::MatrixXd P;      // Points above ground
    Eigen::MatrixXd V;      // Ground coordinates
    std::vector<double> XY; // Ground xy coordinates
    Eigen::MatrixXi F;      // Ground triangles
    Eigen::MatrixXd D;      // List of smallest squared distances
    Eigen::MatrixXi I;      // List of indices to smallest distances
    Eigen::MatrixXd C;      // 3 list of closest points
};

#endif /* ELEVATION_HPP */
