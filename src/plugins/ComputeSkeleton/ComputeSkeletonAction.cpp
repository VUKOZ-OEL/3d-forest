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

/** @file ComputeSkeletonAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <ComputeSkeletonAction.hpp>
#include <ComputeSkeletonMethod.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeSkeletonAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_SKELETON_STEP_POINTS_TO_TREES 0
#define COMPUTE_SKELETON_STEP_CALCULATE_SKELETON 1
#define COMPUTE_SKELETON_STEP_UPDATE_SKELETON 2

ComputeSkeletonAction::ComputeSkeletonAction(Editor *editor)
    : editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeSkeletonAction::~ComputeSkeletonAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeSkeletonAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    treesMap_.clear();
    trees_.clear();
}

void ComputeSkeletonAction::start(const ComputeSkeletonParameters &parameters)
{
    LOG_DEBUG(<< "Start.");

    // Set input parameters.
    double ppm = editor_->settings().unitsSettings().pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.voxelSize *= ppm;

    // Clear work data.
    treesMap_.clear();
    trees_.clear();

    // Plan the steps.
    progress_.setMaximumStep(ProgressCounter::npos, 1000);
    progress_.setMaximumSteps(1);
    progress_.setValueSteps(COMPUTE_SKELETON_STEP_POINTS_TO_TREES);
}

void ComputeSkeletonAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_SKELETON_STEP_POINTS_TO_TREES:
            stepPointsToTrees();
            break;

        case COMPUTE_SKELETON_STEP_CALCULATE_SKELETON:
            stepCalculateComputeSkeleton();
            break;

        case COMPUTE_SKELETON_STEP_UPDATE_SKELETON:
            stepUpdateComputeSkeleton();
            break;

        default:
            // Empty.
            break;
    }
}

size_t ComputeSkeletonAction::treeIndex(size_t treeId, const Segment &segment)
{
    auto it = treesMap_.find(treeId);

    if (it == treesMap_.end())
    {
        size_t index = trees_.size();
        treesMap_[treeId] = index;

        trees_.push_back(ComputeSkeletonData());
        trees_[index].treeId = treeId;
        trees_[index].points.reserve(100);

        return index;
    }

    return it->second;
}

void ComputeSkeletonAction::stepPointsToTrees()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to iterate all points. Active filter is ignored.
        LOG_DEBUG(<< "Start dividing points to trees.");
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    const Segments &segments = editor_->segments();

    // For each point in all datasets:
    while (query_.next())
    {
        size_t treeId = query_.segment();

        if (treeId > 0)
        {
            size_t segmentIndex = segments.index(treeId, false);

            if (segmentIndex != SIZE_MAX)
            {
                const Segment &segment = segments[segmentIndex];

                ComputeSkeletonData &tree = trees_[treeIndex(treeId, segment)];

                tree.points.push_back(query_.x());
                tree.points.push_back(query_.y());
                tree.points.push_back(query_.z());
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next Step.
    if (trees_.size() > 0)
    {
        // Continue.
        LOG_DEBUG(<< "Found <" << trees_.size() << "> trees from points.");
        progress_.setMaximumStep(trees_.size(), 1);
        progress_.setValueSteps(COMPUTE_SKELETON_STEP_CALCULATE_SKELETON);
    }
    else
    {
        // Finish.
        LOG_DEBUG(<< "No trees were found.");
        progress_.setValueStep(progress_.maximumStep());
        progress_.setValueSteps(progress_.maximumSteps());
    }
}

void ComputeSkeletonAction::stepCalculateComputeSkeleton()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Calculating skeleton for <" << trees_.size()
                  << "> trees.");

        currentTreeIndex_ = 0;
    }

    // For each tree:
    while (currentTreeIndex_ < trees_.size())
    {
        ComputeSkeletonData &tree = trees_[currentTreeIndex_];

        calculateSkeleton(tree);

        // Next tree.
        currentTreeIndex_++;
        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next Step.
    progress_.setMaximumStep();
    progress_.setValueSteps(COMPUTE_SKELETON_STEP_UPDATE_SKELETON);
}

void ComputeSkeletonAction::calculateSkeleton(ComputeSkeletonData &tree)
{
    LOG_DEBUG(<< "Calculating skeleton for tree index <"
              << (currentTreeIndex_ + 1) << "/" << trees_.size()
              << "> tree ID <" << tree.treeId << ">.");

    ComputeSkeletonMethod::compute(tree, parameters_);
}

void ComputeSkeletonAction::stepUpdateComputeSkeleton()
{
    LOG_DEBUG(<< "Update <" << trees_.size() << "> trees.");

    // Get copy of current segments.
    Segments segments = editor_->segments();

    // Iterate all groups:
    for (auto &it : trees_)
    {
        Segment &segment = segments[segments.index(it.treeId)];
    }

    // Set new segments to editor.
    editor_->setSegments(segments);

    // Finish.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}
