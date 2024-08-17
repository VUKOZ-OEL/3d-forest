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

/** @file DbhAction.cpp */

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <DbhAction.hpp>
#include <DbhLeastSquaredRegression.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "DbhAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define DBH_STEP_POINTS_TO_GROUPS 0
#define DBH_STEP_CALCULATE_DBH 1
#define DBH_STEP_UPDATE_SEGMENTS 2

DbhAction::DbhAction(Editor *editor) : editor_(editor), query_(editor)
{
    LOG_DEBUG(<< "Create.");
}

DbhAction::~DbhAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void DbhAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();

    groups_.clear();
}

void DbhAction::start(const DbhParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().units.pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.elevation *= ppm;
    parameters_.elevationTolerance *= ppm;

    // Clear work data.
    groups_.clear();

    // Plan the steps.
    progress_.setMaximumStep(ProgressCounter::npos, 1000);
    progress_.setMaximumSteps(1);
    progress_.setValueSteps(DBH_STEP_POINTS_TO_GROUPS);
}

void DbhAction::next()
{
    switch (progress_.valueSteps())
    {
        case DBH_STEP_POINTS_TO_GROUPS:
            stepPointsToGroups();
            break;

        case DBH_STEP_CALCULATE_DBH:
            stepCalculateDbh();
            break;

        case DBH_STEP_UPDATE_SEGMENTS:
            stepUpdateSegments();
            break;

        default:
            // Empty.
            break;
    }
}

void DbhAction::stepPointsToGroups()
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

    // For each point in all datasets:
    while (query_.next())
    {
        // When Z coordinate is within elevation +- elevation tolerance:
        if (query_.z() >
                parameters_.elevation - parameters_.elevationTolerance &&
            query_.z() < parameters_.elevation + parameters_.elevationTolerance)
        {
            size_t segmentId = query_.segment();

            // Find existing group by segmentId of the current point.
            auto it = groups_.find(segmentId);
            if (it == groups_.end())
            {
                // Add new group when it does not exist.
                auto result = groups_.insert({segmentId, DbhGroup()});
                it = result.first;
                it->second.segmentId = segmentId;
                it->second.points.reserve(100);
            }

            // Add point XYZ coordinates to the group.
            it->second.points.push_back(query_.x());
            it->second.points.push_back(query_.y());
            it->second.points.push_back(query_.z());
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
        progress_.setValueStep(groups_.size());
        progress_.setValueSteps(DBH_STEP_CALCULATE_DBH);
    }
    else
    {
        // Finish.
        LOG_DEBUG(<< "No segments were found.");
        progress_.setValueStep(progress_.maximumStep());
        progress_.setValueSteps(progress_.maximumSteps());
    }
}

void DbhAction::stepCalculateDbh()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Calculating DBH for each group.");
        currentGroup_ = 0;
    }

    // For each group:
    while (currentGroup_ < groups_.size())
    {
        // Calculate DBH.
        LOG_DEBUG(<< "Calculating DBH for group <" << currentGroup_ << ">.");

        DbhLeastSquaredRegression::FittingCircle circle;

        DbhLeastSquaredRegression::taubinFit(circle,
                                             groups_[currentGroup_],
                                             parameters_);

        DbhLeastSquaredRegression::geometricCircle(circle,
                                                   groups_[currentGroup_],
                                                   parameters_);

        groups_[currentGroup_].center.set(circle.a, circle.b, circle.z);
        groups_[currentGroup_].radius = circle.r;

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
    progress_.setValueSteps(DBH_STEP_UPDATE_SEGMENTS);
}

void DbhAction::stepUpdateSegments()
{
    LOG_DEBUG(<< "Update <" << groups_.size() << "> segments.");

    // Get copy of current segments.
    Segments segments = editor_->segments();

    // Iterate all groups:
    for (auto &it : groups_)
    {
        Segment &segment = segments[segments.index(it.second.segmentId)];
        segment.position.set(it.second.center[0],
                             it.second.center[1],
                             it.second.center[2]);
        segment.dbh = it.second.radius * 2.0;
    }

    // Set new segments to editor.
    editor_->setSegments(segments);

    // Finish.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void DbhAction::dumpGroups()
{
    LOG_DEBUG(<< "Group count <" << groups_.size() << ">.");
    for (auto const &it : groups_)
    {
        LOG_DEBUG(<< "Group <" << toString(it.second) << ">.");
    }
}
