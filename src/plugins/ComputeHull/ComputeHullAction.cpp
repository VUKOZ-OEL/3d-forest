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

/** @file ComputeHullAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <ComputeHullAction.hpp>
#include <ComputeHullMethod.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeHullAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_HULL_STEP_RESET_POINTS 0
#define COMPUTE_HULL_STEP_COUNT_POINTS 1
#define COMPUTE_HULL_STEP_POINTS_TO_VOXELS 2
#define COMPUTE_HULL_STEP_CALCULATE_HULL 3

ComputeHullAction::ComputeHullAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeHullAction::~ComputeHullAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeHullAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    treesMap_.clear();
    trees_.clear();
}

void ComputeHullAction::start(const ComputeHullParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().unitsSettings().pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.voxelRadius *= ppm;

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;

    treesMap_.clear();
    trees_.clear();

    // Plan the steps.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps({25.0, 25.0, 25.0, 25.0});
    progress_.setValueSteps(COMPUTE_HULL_STEP_RESET_POINTS);
}

void ComputeHullAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_HULL_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_HULL_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_HULL_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case COMPUTE_HULL_STEP_CALCULATE_HULL:
            stepCalculateHull();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeHullAction::stepResetPoints()
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
    progress_.setValueSteps(COMPUTE_HULL_STEP_COUNT_POINTS);
}

void ComputeHullAction::stepCountPoints()
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
    progress_.setValueSteps(COMPUTE_HULL_STEP_POINTS_TO_VOXELS);
}

void ComputeHullAction::stepPointsToVoxels()
{
    progress_.startTimer();

    const Segments &segments = editor_->segments();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point index to voxel is none:
        if (query_.voxel() == SIZE_MAX)
        {
            size_t treeId = query_.segment();
            if (treeId > 0)
            {
                size_t segmentIndex = segments.index(treeId, false);
                if (segmentIndex != SIZE_MAX)
                {
                    const Segment &segment = segments[segmentIndex];
                    double h = query_.z() - segment.boundary.min(2);
                    if (h >= segment.treeAttributes.crownStartHeight)
                    {
                        // Create new voxel.
                        createVoxel();
                    }
                }
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(trees_.size(), 1);
    progress_.setValueSteps(COMPUTE_HULL_STEP_CALCULATE_HULL);
}

void ComputeHullAction::stepCalculateHull()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start calculating hull for <" << trees_.size()
                  << "> trees.");

        currentTreeIndex_ = 0;
    }

    // For each tree:
    while (currentTreeIndex_ < trees_.size())
    {
        LOG_DEBUG(<< "Calculating hull for tree index <"
                  << (currentTreeIndex_ + 1) << "/" << trees_.size()
                  << "> tree ID <" << trees_[currentTreeIndex_].treeId
                  << "> point count <"
                  << trees_[currentTreeIndex_].points.size() / 3 << ">.");

        const ComputeHullData &treeData = trees_[currentTreeIndex_];
        Segment segment = editor_->segment(treeData.treeId);

        if (parameters_.computeConvexHull)
        {
            calculateConvexHull(segment, treeData);
        }

        if (parameters_.computeConvexHullProjection)
        {
            calculateConvexHullProjection(segment, treeData);
        }

        if (parameters_.computeConcaveHull)
        {
            calculateAlphaShape3(segment, treeData);
        }

        if (parameters_.computeConcaveHullProjection)
        {
            calculateAlphaShape2(segment, treeData);
        }

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

    LOG_DEBUG(<< "Finished calculating hull for trees.");
}

void ComputeHullAction::calculateConvexHull(Segment &segment,
                                            const ComputeHullData &data)
{
    Mesh mesh;
    Vector3<double> c;
    ComputeHullMethod::qhull3d(mesh, c, data.points);
    mesh.name = "convexHull";
    segment.treeAttributes.crownCenter = c;
    segment.meshList[mesh.name] = std::move(mesh);
}

void ComputeHullAction::calculateConvexHullProjection(
    Segment &segment,
    const ComputeHullData &data)
{
    float z = static_cast<float>(segment.boundary.min(2));

    Mesh mesh;
    ComputeHullMethod::qhull2d(mesh, data.points, z);
    segment.treeAttributes.surfaceAreaProjection =
        mesh.calculateSurfaceArea2d();
    mesh.name = "convexHullProjection";
    segment.meshList[mesh.name] = std::move(mesh);
}

void ComputeHullAction::calculateAlphaShape3(Segment &segment,
                                             const ComputeHullData &data)
{
    Mesh m;
    double v;
    double s;
    Vector3<double> c;
    ComputeHullMethod::alphaShape3(m, v, s, c, data.points, parameters_.alpha);
    m.name = "concaveHull";
    segment.treeAttributes.volume = v;
    segment.treeAttributes.surfaceArea = s;
    segment.treeAttributes.crownCenter = c;
    segment.meshList[m.name] = std::move(m);
}

void ComputeHullAction::calculateAlphaShape2(Segment &segment,
                                             const ComputeHullData &data)
{
    float z = static_cast<float>(segment.boundary.min(2));

    Mesh mesh;
    ComputeHullMethod::alphaShape2(mesh, data.points, parameters_.alpha, z);
    segment.treeAttributes.surfaceAreaProjection =
        mesh.calculateSurfaceArea2d();
    mesh.name = "concaveHullProjection";
    segment.meshList[mesh.name] = std::move(mesh);
}

size_t ComputeHullAction::treeIndex(size_t treeId)
{
    auto it = treesMap_.find(treeId);

    if (it == treesMap_.end())
    {
        size_t index = trees_.size();
        treesMap_[treeId] = index;
        trees_.push_back(ComputeHullData());
        trees_[index].treeId = treeId;
        trees_[index].points.reserve(100);
        return index;
    }

    return it->second;
}

void ComputeHullAction::createVoxel()
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
    ComputeHullData &tree = trees_[treeIndex(treeId)];
    tree.points.push_back(x);
    tree.points.push_back(y);
    tree.points.push_back(z);
}
