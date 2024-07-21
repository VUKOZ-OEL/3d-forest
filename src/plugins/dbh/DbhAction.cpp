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
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "DbhAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define DBH_STEP_POINTS_TO_GROUPS 0

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

    parameters_.elevationMin *= ppm;
    parameters_.elevationMax *= ppm;

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

        default:
            // Empty.
            break;
    }
}

void DbhAction::stepPointsToGroups()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        // Set query to iterate all points. Active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // For each point in all datasets:
    while (query_.next())
    {
        if (query_.z() > parameters_.elevationMin &&
            query_.z() < parameters_.elevationMax)
        {
            size_t segmentId = query_.segment();

            auto it = groups_.find(segmentId);
            if (it == groups_.end())
            {
                auto result = groups_.insert({segmentId, Group()});
                it = result.first;
                it->second.points.reserve(100);
            }

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

    LOG_DEBUG(<< "Groups <" << groups_.size() << ">.");
    for (auto const &it : groups_)
    {
        LOG_DEBUG(<< "segmentId <" << it.second.segmentId << "> point count <"
                  << it.second.points.size() << ">.");
    }

    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}
