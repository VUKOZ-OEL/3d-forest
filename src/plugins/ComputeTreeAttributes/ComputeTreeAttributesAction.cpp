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

/** @file ComputeTreeAttributesAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <ComputeTreeAttributesAction.hpp>
#include <ComputeTreeAttributesLeastSquaredRegression.hpp>
#include <ComputeTreeAttributesRandomizedHoughTransform.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeTreeAttributesAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_TREE_ATTRIBUTES_STEP_POINTS_TO_TREES 0
#define COMPUTE_TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES 1
#define COMPUTE_TREE_ATTRIBUTES_STEP_UPDATE_TREE_ATTRIBUTES 2

ComputeTreeAttributesAction::ComputeTreeAttributesAction(Editor *editor)
    : editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeTreeAttributesAction::~ComputeTreeAttributesAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeTreeAttributesAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    treesMap_.clear();
    trees_.clear();
}

void ComputeTreeAttributesAction::start(
    const ComputeTreeAttributesParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().unitsSettings().pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.treePositionHeightRange *= ppm;
    parameters_.treeTipHeightRange *= ppm;
    parameters_.dbhElevation *= ppm;
    parameters_.dbhElevationRange *= ppm;
    parameters_.maximumValidCalculatedDbh *= ppm;

    // Clear work data.
    treesMap_.clear();
    trees_.clear();

    // Plan the steps.
    progress_.setMaximumStep(ProgressCounter::npos, 1000);
    progress_.setMaximumSteps(1);
    progress_.setValueSteps(COMPUTE_TREE_ATTRIBUTES_STEP_POINTS_TO_TREES);
}

void ComputeTreeAttributesAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_TREE_ATTRIBUTES_STEP_POINTS_TO_TREES:
            stepPointsToTrees();
            break;

        case COMPUTE_TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES:
            stepCalculateComputeTreeAttributes();
            break;

        case COMPUTE_TREE_ATTRIBUTES_STEP_UPDATE_TREE_ATTRIBUTES:
            stepUpdateComputeTreeAttributes();
            break;

        default:
            // Empty.
            break;
    }
}

size_t ComputeTreeAttributesAction::treeIndex(size_t treeId)
{
    auto it = treesMap_.find(treeId);

    if (it == treesMap_.end())
    {
        size_t index = trees_.size();
        treesMap_[treeId] = index;
        trees_.push_back(ComputeTreeAttributesData());
        trees_[index].treeId = treeId;
        trees_[index].dbhPoints.reserve(100);
        return index;
    }

    return it->second;
}

void ComputeTreeAttributesAction::stepPointsToTrees()
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

        if (treeId > 0 && treeId < segments.size())
        {
            const Segment &segment = segments[treeId];

            // When point Z distance from the minimal tree boundary Z value
            // is within tree position range, then:
            if ((query_.z() - segment.boundary.min(2)) <=
                parameters_.treePositionHeightRange)
            {
                ComputeTreeAttributesData &tree = trees_[treeIndex(treeId)];

                // Add point X and Y coordinates to X and Y coordinates lists.
                tree.xCoordinates.push_back(query_.x());
                tree.yCoordinates.push_back(query_.y());

                // When point Z coordinate has new lowest value, then
                // set the value as new Z minimum.
                if (query_.z() < tree.zCoordinateMin)
                {
                    tree.zCoordinateMin = query_.z();
                }
            }

            // When point elevation is within DBH elevation range, then
            // add point XYZ coordinates to DBH point list.
            if (query_.elevation() >=
                    parameters_.dbhElevation - parameters_.dbhElevationRange &&
                query_.elevation() <=
                    parameters_.dbhElevation + parameters_.dbhElevationRange)
            {
                ComputeTreeAttributesData &tree = trees_[treeIndex(treeId)];

                tree.dbhPoints.push_back(query_.x());
                tree.dbhPoints.push_back(query_.y());
                tree.dbhPoints.push_back(query_.z());
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
        progress_.setValueSteps(
            COMPUTE_TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES);
    }
    else
    {
        // Finish.
        LOG_DEBUG(<< "No trees were found.");
        progress_.setValueStep(progress_.maximumStep());
        progress_.setValueSteps(progress_.maximumSteps());
    }
}

void ComputeTreeAttributesAction::stepCalculateComputeTreeAttributes()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Calculating tree attributes for <" << trees_.size()
                  << "> trees.");

        currentTreeIndex_ = 0;
    }

    // For each tree:
    while (currentTreeIndex_ < trees_.size())
    {
        LOG_DEBUG(<< "Calculating tree attributes for tree index <"
                  << (currentTreeIndex_ + 1) << "/" << trees_.size()
                  << "> tree ID <" << trees_[currentTreeIndex_].treeId << ">.");

        // Calculate DBH.
        calculateDbh(trees_[currentTreeIndex_]);

        // Calculate tree position.
        calculateTreePosition(trees_[currentTreeIndex_]);

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
    progress_.setValueSteps(
        COMPUTE_TREE_ATTRIBUTES_STEP_UPDATE_TREE_ATTRIBUTES);
}

void ComputeTreeAttributesAction::calculateDbh(ComputeTreeAttributesData &tree)
{
    switch (parameters_.dbhMethod)
    {
        case ComputeTreeAttributesParameters::DbhMethod::RHT:
            calculateDbhRht(tree);
            break;
        case ComputeTreeAttributesParameters::DbhMethod::LSR:
            calculateDbhLsr(tree);
            break;
        default:
            THROW("Unknown DBH method");
            break;
    }
}

void ComputeTreeAttributesAction::calculateDbhRht(
    ComputeTreeAttributesData &tree)
{
    ComputeTreeAttributesRandomizedHoughTransform::FittingCircle circle;

    ComputeTreeAttributesRandomizedHoughTransform::compute(circle,
                                                           tree.dbhPoints,
                                                           parameters_);

    tree.treeAttributes.dbhPosition.set(circle.a, circle.b, circle.z);
    tree.treeAttributes.dbhNormal.set(0.0, 0.0, 1.0);
    tree.treeAttributes.dbh = circle.r * 2.0;
}

void ComputeTreeAttributesAction::calculateDbhLsr(
    ComputeTreeAttributesData &tree)
{
    ComputeTreeAttributesLeastSquaredRegression::FittingCircle circle;

    ComputeTreeAttributesLeastSquaredRegression::taubinFit(circle,
                                                           tree.dbhPoints,
                                                           parameters_);

    ComputeTreeAttributesLeastSquaredRegression::geometricCircle(circle,
                                                                 tree.dbhPoints,
                                                                 parameters_);

    tree.treeAttributes.dbhPosition.set(circle.a, circle.b, circle.z);
    tree.treeAttributes.dbhNormal.set(0.0, 0.0, 1.0);
    tree.treeAttributes.dbh = circle.r * 2.0;
}

void ComputeTreeAttributesAction::calculateTreePosition(
    ComputeTreeAttributesData &tree)
{
    LOG_DEBUG(<< "Tree position x coordinates size <"
              << tree.xCoordinates.size() << ">.");
    LOG_DEBUG(<< "Tree position y coordinates size <"
              << tree.yCoordinates.size() << ">.");
    LOG_DEBUG(<< "Tree position z coordinate min <" << tree.zCoordinateMin
              << ">.");

    std::sort(tree.xCoordinates.begin(), tree.xCoordinates.end());
    std::sort(tree.yCoordinates.begin(), tree.yCoordinates.end());

    double x;
    if (tree.xCoordinates.size() > 0)
    {
        x = tree.xCoordinates.at(tree.xCoordinates.size() / 2);
    }
    else
    {
        x = 0.0;
    }

    double y;
    if (tree.yCoordinates.size() > 0)
    {
        y = tree.yCoordinates.at(tree.yCoordinates.size() / 2);
    }
    else
    {
        y = 0.0;
    }

    double z;
    if (tree.zCoordinateMin < Numeric::max<double>())
    {
        z = tree.zCoordinateMin;
    }
    else
    {
        z = 0.0;
    }

    tree.treeAttributes.position.set(x, y, z);
}

void ComputeTreeAttributesAction::stepUpdateComputeTreeAttributes()
{
    LOG_DEBUG(<< "Update <" << trees_.size() << "> trees.");

    // Get copy of current segments.
    Segments segments = editor_->segments();

    // Iterate all groups:
    for (auto &it : trees_)
    {
        Segment &segment = segments[segments.index(it.treeId)];

        segment.treeAttributes = it.treeAttributes;
        segment.treeAttributes.height = segment.boundary.length(2);
        validateAttributes(segment.treeAttributes);

        LOG_DEBUG(<< "Tree position <" << segment.treeAttributes.position
                  << "> height <" << segment.treeAttributes.height << "> DBH <"
                  << segment.treeAttributes.dbh << ">.");
    }

    // Set new segments to editor.
    editor_->setSegments(segments);

    // Finish.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void ComputeTreeAttributesAction::validateAttributes(
    TreeAttributes &treeAttributes)
{
    if (treeAttributes.dbh > parameters_.maximumValidCalculatedDbh)
    {
        treeAttributes.dbh = 0.0;
    }
}
