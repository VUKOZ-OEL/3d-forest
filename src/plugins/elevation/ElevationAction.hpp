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

/** @file ElevationAction.hpp */

#ifndef ELEVATION_ACTION_HPP
#define ELEVATION_ACTION_HPP

#include <Eigen/Core>

#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Elevation Action. */
class ElevationAction : public ProgressActionInterface
{
public:
    ElevationAction(Editor *editor);
    virtual ~ElevationAction();

    void initialize(size_t pointsPerCell = 10000,
                    double cellLengthMinPercent = 1.);
    virtual void next();
    void exportGroundMesh(const std::string &path);
    void clear();

    double minimum() const { return elevationMinimum_; }
    double maximum() const { return elevationMaximum_; }

protected:
    Editor *editor_;
    Query query_;

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

    void stepGrid();
};

#endif /* ELEVATION_ACTION_HPP */
