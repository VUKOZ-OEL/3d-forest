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

/** @file SegmentationAction.hpp */

#ifndef SEGMENTATION_ACTION_HPP
#define SEGMENTATION_ACTION_HPP

#include <Point.hpp>
#include <Points.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;

/** Segmentation Action. */
class SegmentationAction : public ProgressActionInterface
{
public:
    SegmentationAction(Editor *editor);
    virtual ~SegmentationAction();

    void start(double voxelSize,
               double descriptor,
               double trunkRadius,
               double leafRadius,
               double elevationMin,
               double elevationMax,
               double treeHeight,
               bool useZ,
               bool onlyTrunks);

    virtual void next();

    void clear();

private:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    double voxelSize_;
    double descriptor_;
    double trunkRadius_;
    double leafRadius_;
    double elevationMin_;
    double elevationMax_;
    double treeHeight_;
    bool useZ_;
    bool onlyTrunks_;

    uint64_t nPointsTotal_;
    uint64_t nPointsInFilter_;

    void stepResetPoints();
    void stepCountPoints();
    void stepPointsToVoxels();
    void stepCreateVoxelIndex();
    void stepCreateTrees();
    void stepConnectVoxels();
    void stepCreateLayers();
    void stepVoxelsToPoints();

    void createVoxel();
    void findNearestNeighbor(Point &a);
    bool isTrunkVoxel(const Point &a);
    void startGroup(const Point &a);
    void continueGroup(const Point &a);

    Points voxels_;
    std::map<size_t, size_t> groups_;
    std::vector<size_t> path_;
    std::vector<size_t> group_;
    std::vector<size_t> search_;
    size_t pointIndex_;
    size_t groupId_;
    double groupMinimum_;
    double groupMaximum_;
};

#endif /* SEGMENTATION_ACTION_HPP */
