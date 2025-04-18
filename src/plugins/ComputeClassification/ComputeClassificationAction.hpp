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

/** @file ComputeClassificationAction.hpp */

#ifndef COMPUTE_CLASSIFICATION_ACTION_HPP
#define COMPUTE_CLASSIFICATION_ACTION_HPP

// Include 3D Forest.
#include <ComputeClassificationParameters.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;

/** Compute Classification Action. */
class ComputeClassificationAction : public ProgressActionInterface
{
public:
    ComputeClassificationAction(Editor *editor);
    virtual ~ComputeClassificationAction();

    void start(const ComputeClassificationParameters &parameters);
    virtual void next();
    void clear();

protected:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    ComputeClassificationParameters parameters_;

    uint64_t nPointsTotal_;
    uint64_t nPointsInFilter_;

    void stepResetPoints();
    void stepCountPoints();
    void stepPointsToVoxels();
    void stepCreateVoxelIndex();
    void stepClassifyGround();
    void stepVoxelsToPoints();

    void createVoxel();

    Points voxels_;
    std::vector<size_t> group_;
    std::vector<size_t> path_;
    std::vector<size_t> searchNext_;
    std::vector<size_t> searchGround_;

    size_t minimumIndex_;
    double minimumValue_;
};

#endif /* COMPUTE_CLASSIFICATION_ACTION_HPP */
