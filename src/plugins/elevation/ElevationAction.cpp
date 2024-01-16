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

/** @file ElevationAction.cpp */

#include <Editor.hpp>
#include <ElevationAction.hpp>

#define LOG_MODULE_NAME "ElevationAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ELEVATION_STEP_RESET_POINTS 0
#define ELEVATION_STEP_COUNT_POINTS 1
#define ELEVATION_STEP_CREATE_GROUND 2
#define ELEVATION_STEP_CREATE_INDEX 3
#define ELEVATION_STEP_COMPUTE_ELEVATION 4

ElevationAction::ElevationAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ElevationAction::~ElevationAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ElevationAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    voxelSize_ = 0;

    numberOfPoints_ = 0;
    numberOfGroundPoints_ = 0;
    numberOfNonGroundPoints_ = 0;
    pointIndex_ = 0;

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    points_.clear();
}

void ElevationAction::start(double voxelSize)
{
    LOG_DEBUG(<< "Start voxelSize <" << voxelSize << ">.");

    voxelSize_ = voxelSize;

    numberOfPoints_ = editor_->datasets().nPoints();
    numberOfGroundPoints_ = 0;
    numberOfNonGroundPoints_ = 0;
    pointIndex_ = 0;
    LOG_DEBUG(<< "Total number of points <" << numberOfPoints_ << ">.");

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    points_.clear();

    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setMaximumSteps({5.0, 5.0, 25.0, 5.0, 60.0});
    progress_.setValueSteps(ELEVATION_STEP_RESET_POINTS);
}

void ElevationAction::next()
{
    switch (progress_.valueSteps())
    {
        case ELEVATION_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case ELEVATION_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case ELEVATION_STEP_CREATE_GROUND:
            stepCreateGround();
            break;

        case ELEVATION_STEP_CREATE_INDEX:
            stepCreateIndex();
            break;

        case ELEVATION_STEP_COMPUTE_ELEVATION:
            stepComputeElevation();
            break;

        default:
            // empty
            break;
    }
}

void ElevationAction::stepResetPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Reset elevation range.
        Range<double> range;
        editor_->setElevationFilter(range);

        // Set query to iterate all points. The active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // Clear each point in all datasets.
    while (query_.next())
    {
        query_.voxel() = 0;
        query_.elevation() = 0;
        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setValueSteps(ELEVATION_STEP_COUNT_POINTS);
}

void ElevationAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to use the active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    // Iterate all filtered points.
    while (query_.next())
    {
        if (query_.classification() == LasFile::CLASS_GROUND)
        {
            numberOfGroundPoints_++;
        }
        else
        {
            numberOfNonGroundPoints_++;
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfGroundPoints_, 100);
    progress_.setValueSteps(ELEVATION_STEP_CREATE_GROUND);
}

void ElevationAction::stepCreateGround()
{
    progress_.startTimer();

    // Iterate all points:
    while (query_.next())
    {
        if (query_.classification() == LasFile::CLASS_GROUND)
        {
            // If the current point is classified as ground,
            // then add it as new ground point.
            createGroundPoint();
            progress_.addValueStep(1);
        }

        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    progress_.setMaximumStep();
    progress_.setValueSteps(ELEVATION_STEP_CREATE_INDEX);
}

void ElevationAction::stepCreateIndex()
{
    // Create ground index.
    points_.createIndex();

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfNonGroundPoints_, 100);
    progress_.setValueSteps(ELEVATION_STEP_COMPUTE_ELEVATION);
}

void ElevationAction::stepComputeElevation()
{
    progress_.startTimer();

    // Iterate all points:
    while (query_.next())
    {
        // If the current point is not classified as ground:
        if (query_.classification() != LasFile::CLASS_GROUND)
        {
            // Find nearest neighbour in ground projection:
            size_t idx = points_.findNN(query_.x(), query_.y(), 0.0);
            if (idx < points_.size())
            {
                // Compute elevation to this nearest neighbour.
                double d = query_.z() - points_[idx].elevation;
                if (d < 0.0)
                {
                    d = 0.0;
                }

                // Update min and max elevation.
                if (pointIndex_ == 0)
                {
                    elevationMinimum_ = d;
                    elevationMaximum_ = d;
                }
                else if (d < elevationMinimum_)
                {
                    elevationMinimum_ = d;
                }
                else if (d > elevationMaximum_)
                {
                    elevationMaximum_ = d;
                }
                pointIndex_++;

                // Set computed elevation.
                query_.elevation() = d;
            }

            progress_.addValueStep(1);
        }

        if (progress_.timedOut())
        {
            return;
        }
    }

    // Flush all modifications.
    query_.flush();

    // Set new elevation range.
    Range<double> range;
    range.set(elevationMinimum_, elevationMaximum_);
    editor_->setElevationFilter(range);

    // All steps are now complete.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void ElevationAction::createGroundPoint()
{
    // If this ground point was already processed, then do nothing.
    if (query_.voxel() != 0)
    {
        return;
    }

    // Initialize new ground point.
    Point p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    p.elevation = 0;

    // Compute point coordinates as average from all neighbour points:
    size_t n = 0;

    queryPoint_.where().setSphere(query_.x(),
                                  query_.y(),
                                  query_.z(),
                                  voxelSize_);
    queryPoint_.exec();

    while (queryPoint_.next())
    {
        // Compute only with ground points:
        if (queryPoint_.classification() == LasFile::CLASS_GROUND)
        {
            // Set only x and y point coordinates for 2D ground projection.
            p.x += queryPoint_.x();
            p.y += queryPoint_.y();

            // Store maximal z coordinate from all neighbors into elevation.
            if (queryPoint_.z() > p.elevation)
            {
                p.elevation = queryPoint_.z();
            }

            n++;

            // Mark all used points as processed.
            queryPoint_.voxel() = 1;
            queryPoint_.setModified();
        }
    }

    if (n < 1)
    {
        return;
    }

    p.x = p.x / static_cast<double>(n);
    p.y = p.y / static_cast<double>(n);

    // Append new point.
    points_.push_back(std::move(p));
}
