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

/** @file ComputeConvexHullAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <ComputeConvexHullAction.hpp>
#include <ComputeConvexHullMethod.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeConvexHullAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_CONVEX_HULL_STEP_RESET_POINTS 0
#define COMPUTE_CONVEX_HULL_STEP_COUNT_POINTS 1
#define COMPUTE_CONVEX_HULL_STEP_POINTS_TO_VOXELS 2
#define COMPUTE_CONVEX_HULL_STEP_CALCULATE_HULL 3

ComputeConvexHullAction::ComputeConvexHullAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeConvexHullAction::~ComputeConvexHullAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeConvexHullAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    treesMap_.clear();
    trees_.clear();
}

void ComputeConvexHullAction::start(
    const ComputeConvexHullParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().units().pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.voxelRadius *= ppm;

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;

    treesMap_.clear();
    trees_.clear();

    // Plan the steps.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps(1);
    progress_.setValueSteps(COMPUTE_CONVEX_HULL_STEP_RESET_POINTS);
}

void ComputeConvexHullAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_CONVEX_HULL_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_CONVEX_HULL_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_CONVEX_HULL_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case COMPUTE_CONVEX_HULL_STEP_CALCULATE_HULL:
            stepCalculateHull();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeConvexHullAction::stepResetPoints()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Reset all <" << nPointsTotal_ << "> points.");

        // Set query to iterate all points. Active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // For each point in all datasets:
    while (query_.next())
    {
        // Set point index to voxel to none.
        query_.voxel() = SIZE_MAX;
        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setValueSteps(COMPUTE_CONVEX_HULL_STEP_COUNT_POINTS);
}

void ComputeConvexHullAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to use active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    // Count the number of filtered points.
    while (query_.next())
    {
        nPointsInFilter_++;

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Counted <" << nPointsInFilter_ << "> points in filter.");

    query_.reset();

    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_CONVEX_HULL_STEP_POINTS_TO_VOXELS);
}

void ComputeConvexHullAction::stepPointsToVoxels()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point index to voxel is none:
        if (query_.voxel() == SIZE_MAX && query_.segment() > 0)
        {
            // Create new voxel.
            createVoxel();
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(trees_.size(), 1);
    progress_.setValueSteps(COMPUTE_CONVEX_HULL_STEP_CALCULATE_HULL);
}

void ComputeConvexHullAction::stepCalculateHull()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start calculating convex hull for <" << trees_.size()
                  << "> trees.");

        currentTreeIndex_ = 0;
    }

    // For each tree:
    while (currentTreeIndex_ < trees_.size())
    {
        LOG_DEBUG(<< "Calculating convex hull for tree index <"
                  << (currentTreeIndex_ + 1) << "/" << trees_.size()
                  << "> tree ID <" << trees_[currentTreeIndex_].treeId
                  << "> point count <"
                  << trees_[currentTreeIndex_].points.size() / 3 << ">.");

        Mesh m;
        ComputeConvexHullMethod::quickhull(trees_[currentTreeIndex_].points, m);

        LOG_DEBUG(<< "Calculated convex hull has <" << m.xyz.size() / 3
                  << "> vertices and <" << m.indices.size() / 3
                  << "> triangles.");

        Segment segment = editor_->segment(trees_[currentTreeIndex_].treeId);
        segment.meshList.clear();
        segment.meshList.push_back(std::move(m));
        editor_->setSegment(segment);

        // Next tree.
        currentTreeIndex_++;
        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());

    LOG_DEBUG(<< "Finished calculating convex hull for trees.");
}

size_t ComputeConvexHullAction::treeIndex(size_t treeId)
{
    auto it = treesMap_.find(treeId);

    if (it == treesMap_.end())
    {
        size_t index = trees_.size();
        treesMap_[treeId] = index;
        trees_.push_back(ComputeConvexHullData());
        trees_[index].treeId = treeId;
        trees_[index].points.reserve(100);
        return index;
    }

    return it->second;
}

void ComputeConvexHullAction::createVoxel()
{
    // Initialize new voxel point.
    size_t treeId = query_.segment();
    double x = 0;
    double y = 0;
    double z = 0;

    // Compute point coordinates as average from all neighbour points.
    // Set value of each neighbour point to index of new voxel.
    size_t n = 0;

    queryPoint_.where().setSphere(query_.x(),
                                  query_.y(),
                                  query_.z(),
                                  parameters_.voxelRadius);
    queryPoint_.exec();

    while (queryPoint_.next())
    {
        if (queryPoint_.segment() != treeId)
        {
            continue;
        }

        x += queryPoint_.x();
        y += queryPoint_.y();
        z += queryPoint_.z();

        n++;

        queryPoint_.voxel() = 1;
        queryPoint_.setModified();
    }

    if (n < 1)
    {
        return;
    }

    x = x / static_cast<double>(n);
    y = y / static_cast<double>(n);
    z = z / static_cast<double>(n);

    // Append new voxel to voxel array.
    ComputeConvexHullData &tree = trees_[treeIndex(treeId)];
    tree.points.push_back(x);
    tree.points.push_back(y);
    tree.points.push_back(z);
}
