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

/** @file ComputeSkeletonAction.hpp */

#ifndef COMPUTE_SKELETON_ACTION_HPP
#define COMPUTE_SKELETON_ACTION_HPP

// Include 3D Forest.
#include <ComputeSkeletonData.hpp>
#include <ComputeSkeletonParameters.hpp>
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;
class Segment;

/** Compute Skeleton Action. */
class ComputeSkeletonAction : public ProgressActionInterface
{
public:
    ComputeSkeletonAction(Editor *editor);
    virtual ~ComputeSkeletonAction();

    void start(const ComputeSkeletonParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;

    ComputeSkeletonParameters parameters_;

    std::map<size_t, size_t> treesMap_; // [tree ID : tree index]
    std::vector<ComputeSkeletonData> trees_;

    size_t currentTreeIndex_;

    void stepPointsToTrees();
    void stepCalculateComputeSkeleton();
    void stepUpdateComputeSkeleton();

    void calculateSkeleton(ComputeSkeletonData &tree);

    size_t treeIndex(size_t treeId, const Segment &segment);
};

#endif /* COMPUTE_SKELETON_ACTION_HPP */
