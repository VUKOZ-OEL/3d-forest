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

/** @file TreeAttributesAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <TreeAttributesAction.hpp>
#include <TreeAttributesLeastSquaredRegression.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeAttributesAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define TREE_ATTRIBUTES_STEP_POINTS_TO_GROUPS 0
#define TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES 1
#define TREE_ATTRIBUTES_STEP_UPDATE_SEGMENTS 2

TreeAttributesAction::TreeAttributesAction(Editor *editor)
    : editor_(editor),
      query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

TreeAttributesAction::~TreeAttributesAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void TreeAttributesAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();

    groups_.clear();
}

void TreeAttributesAction::start(const TreeAttributesParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().units.pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.treePositionZTolerance *= ppm;
    parameters_.dbhElevation *= ppm;
    parameters_.dbhElevationTolerance *= ppm;

    // Clear work data.
    groups_.clear();

    // Plan the steps.
    progress_.setMaximumStep(ProgressCounter::npos, 1000);
    progress_.setMaximumSteps(1);
    progress_.setValueSteps(TREE_ATTRIBUTES_STEP_POINTS_TO_GROUPS);
}

void TreeAttributesAction::next()
{
    switch (progress_.valueSteps())
    {
        case TREE_ATTRIBUTES_STEP_POINTS_TO_GROUPS:
            stepPointsToGroups();
            break;

        case TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES:
            stepCalculateTreeAttributes();
            break;

        case TREE_ATTRIBUTES_STEP_UPDATE_SEGMENTS:
            stepUpdateSegments();
            break;

        default:
            // Empty.
            break;
    }
}

void TreeAttributesAction::stepPointsToGroups()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to iterate all points. Active filter is ignored.
        LOG_DEBUG(<< "Start dividing points to groups.");
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    const Segments &segments = editor_->segments();

    // For each point in all datasets:
    while (query_.next())
    {
        size_t segmentId = query_.segment();

        bool pointIsInTreeBaseRange = false;
        if (segmentId > 0 && segmentId < segments.size())
        {
            const Segment &segment = segments[segmentId];
            if ((query_.z() - segment.boundary.min(2)) <
                parameters_.treePositionZTolerance)
            {
                LOG_DEBUG(<< "Point z <" << query_.z()
                          << "> is in base range of segment id <" << segmentId
                          << ">.");
                pointIsInTreeBaseRange = true;
            }
        }

        bool pointIsInDbhRange =
            query_.elevation() >
                parameters_.dbhElevation - parameters_.dbhElevationTolerance &&
            query_.elevation() <
                parameters_.dbhElevation + parameters_.dbhElevationTolerance;

        // When
        // point z distance from
        // or
        // point elevation is within elevation +- elevation tolerance:
        if (pointIsInTreeBaseRange || pointIsInDbhRange)
        {
            // Find existing group by segmentId of the current point.
            auto it = groups_.find(segmentId);
            if (it == groups_.end())
            {
                // Add new group when it does not exist.
                auto result =
                    groups_.insert({segmentId, TreeAttributesGroup()});
                it = result.first;
                it->second.segmentId = segmentId;
                it->second.dbhPoints.reserve(100);
            }

            // When point elevation is within DBH elevation range, then
            // add point XYZ coordinates to DBH point set.
            if (pointIsInDbhRange)
            {
                it->second.dbhPoints.push_back(query_.x());
                it->second.dbhPoints.push_back(query_.y());
                it->second.dbhPoints.push_back(query_.z());
            }

            if (pointIsInTreeBaseRange)
            {
                it->second.xCoordinates.push_back(query_.x());
                it->second.yCoordinates.push_back(query_.y());

                if (query_.z() < it->second.zCoordinateMin)
                {
                    it->second.zCoordinateMin = query_.z();
                }
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // dumpGroups();

    // Next Step.
    if (groups_.size() > 0)
    {
        // Continue.
        progress_.setMaximumStep(groups_.size(), 1);
        progress_.setValueSteps(TREE_ATTRIBUTES_STEP_CALCULATE_TREE_ATTRIBUTES);
    }
    else
    {
        // Finish.
        LOG_DEBUG(<< "No segments were found.");
        progress_.setValueStep(progress_.maximumStep());
        progress_.setValueSteps(progress_.maximumSteps());
    }
}

void TreeAttributesAction::stepCalculateTreeAttributes()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Calculating tree attributes for each group.");
        currentGroup_ = 0;
    }

    // For each group:
    while (currentGroup_ < groups_.size())
    {
        LOG_DEBUG(<< "Calculating tree attributes for group <" << currentGroup_
                  << ">.");

        // Calculate DBH.
        calculateDbh(groups_[currentGroup_]);

        // Calculate tree base.
        calculateTreeBase(groups_[currentGroup_]);

        // Next group.
        currentGroup_++;
        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    dumpGroups();

    // Next Step.
    progress_.setMaximumStep();
    progress_.setValueSteps(TREE_ATTRIBUTES_STEP_UPDATE_SEGMENTS);
}

void TreeAttributesAction::calculateDbh(TreeAttributesGroup &group)
{
    TreeAttributesLeastSquaredRegression::FittingCircle circle;

    TreeAttributesLeastSquaredRegression::taubinFit(circle, group, parameters_);

    TreeAttributesLeastSquaredRegression::geometricCircle(circle,
                                                          group,
                                                          parameters_);

    group.dbhCenter.set(circle.a, circle.b, circle.z);
    group.dbh = circle.r * 2.0;
}

void TreeAttributesAction::calculateTreeBase(TreeAttributesGroup &group)
{
    std::sort(group.xCoordinates.begin(), group.xCoordinates.end());
    std::sort(group.yCoordinates.begin(), group.yCoordinates.end());

    LOG_DEBUG(<< "Group size x <" << group.xCoordinates.size() << "> size y <"
              << group.yCoordinates.size() << ">.");

    if (group.xCoordinates.size() > 1 && group.yCoordinates.size() > 1)
    {
        group.position.set(group.xCoordinates.at(group.xCoordinates.size() / 2),
                           group.yCoordinates.at(group.yCoordinates.size() / 2),
                           group.zCoordinateMin);
    }
}

void TreeAttributesAction::stepUpdateSegments()
{
    LOG_DEBUG(<< "Update <" << groups_.size() << "> segments.");

    // Get copy of current segments.
    Segments segments = editor_->segments();

    // Iterate all groups:
    for (auto &it : groups_)
    {
        Segment &segment = segments[segments.index(it.second.segmentId)];
        segment.position.set(it.second.position[0],
                             it.second.position[1],
                             it.second.position[2]);
        segment.dbh = it.second.dbh;

        LOG_DEBUG(<< "Segment position <" << segment.position << "> DBH <"
                  << segment.dbh << ">.");
    }

    // Set new segments to editor.
    editor_->setSegments(segments);

    // Finish.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void TreeAttributesAction::dumpGroups()
{
    LOG_DEBUG(<< "Group count <" << groups_.size() << ">.");
    for (auto const &it : groups_)
    {
        LOG_DEBUG(<< "Group <" << toString(it.second) << ">.");
    }
}
