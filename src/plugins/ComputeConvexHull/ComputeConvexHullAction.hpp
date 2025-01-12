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

/** @file ComputeConvexHullAction.hpp */

#ifndef COMPUTE_CONVEX_HULL_ACTION_HPP
#define COMPUTE_CONVEX_HULL_ACTION_HPP

// Include 3D Forest.
#include <ComputeConvexHullData.hpp>
#include <ComputeConvexHullParameters.hpp>
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;
class Segment;

/** Compute Convex Hull Action. */
class ComputeConvexHullAction : public ProgressActionInterface
{
public:
    ComputeConvexHullAction(Editor *editor);
    virtual ~ComputeConvexHullAction();

    void start(const ComputeConvexHullParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    ComputeConvexHullParameters parameters_;

    uint64_t nPointsTotal_;
    uint64_t nPointsInFilter_;

    std::map<size_t, size_t> treesMap_; // [tree ID : tree index]
    std::vector<ComputeConvexHullData> trees_;

    size_t currentTreeIndex_;

    void stepResetPoints();
    void stepCountPoints();
    void stepPointsToVoxels();
    void stepCalculateHull();

    size_t treeIndex(size_t treeId);

    void createVoxel();
};

#endif /* COMPUTE_CONVEX_HULL_ACTION_HPP */
