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

/** @file ComputeSegmentationNNAction.hpp */

#ifndef COMPUTE_SEGMENTATION_NN_ACTION_HPP
#define COMPUTE_SEGMENTATION_NN_ACTION_HPP

// Include 3D Forest.
#include <ComputeSegmentationNNParameters.hpp>
#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;
class Segment;

/** Compute Segmentation NN Action. */
class ComputeSegmentationNNAction : public ProgressActionInterface
{
public:
    ComputeSegmentationNNAction(Editor *editor);
    virtual ~ComputeSegmentationNNAction();

    void start(const ComputeSegmentationNNParameters &parameters);
    virtual void next();
    void clear();

private:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    ComputeSegmentationNNParameters parameters_;

    uint64_t nPointsTotal_;
    uint64_t nPointsInFilter_;

    void stepResetPoints();
    void stepCountPoints();
    void stepPointsToVoxels();
    void stepCreateVoxelIndex();
    void stepCreateTrunks();
    void stepCreateBranches();
    void stepCreateSegments();
    void stepVoxelsToPoints();

    void createVoxel();
    void findNearestNeighbor(Point &a);
    bool trunkVoxel(const Point &a);

    Points voxels_;
    std::vector<size_t> path_;
    std::vector<size_t> groupPath_;
    std::vector<size_t> search_;
    size_t pointIndex_;
    size_t groupId_;
    double groupMinimum_;
    double groupMaximum_;

    /** ComputeSegmentationNN Group. */
    class Group
    {
    public:
        size_t segmentId = 0;
        size_t nPoints = 0;
        Box<double> boundary;
        Vector3<double> averagePoint;

        void clear();
    };

    Group group_;
    Group groupUnsegmented_;
    std::map<size_t, Group> groups_;

    void startGroup(const Point &a, bool trunk = false);
    void continueGroup(const Point &a, bool trunk = false);
    void mergeToGroup(Group &dst, const Group &src);
};

#endif /* COMPUTE_SEGMENTATION_NN_ACTION_HPP */
