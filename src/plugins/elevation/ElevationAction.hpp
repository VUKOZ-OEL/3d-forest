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

// Include 3D Forest.
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;

/** Elevation Action. */
class ElevationAction : public ProgressActionInterface
{
public:
    ElevationAction(Editor *editor);
    virtual ~ElevationAction();

    void start(double voxelRadius);
    virtual void next();
    void clear();

    double minimum() const { return elevationMinimum_; }
    double maximum() const { return elevationMaximum_; }

protected:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    double voxelRadius_;

    uint64_t numberOfPoints_;
    uint64_t numberOfGroundPoints_;
    uint64_t numberOfNonGroundPoints_;
    size_t pointIndex_;

    double elevationMinimum_;
    double elevationMaximum_;

    Points points_;

    void stepResetPoints();
    void stepCountPoints();
    void stepCreateGround();
    void stepCreateIndex();
    void stepComputeElevation();

    void createGroundPoint();
};

#endif /* ELEVATION_ACTION_HPP */
