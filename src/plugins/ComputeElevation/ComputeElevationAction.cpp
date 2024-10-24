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

/** @file ComputeElevationAction.cpp */

// Include 3D Forest.
#include <ComputeElevationAction.hpp>
#include <Editor.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeElevationAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_ELEVATION_STEP_RESET_POINTS 0
#define COMPUTE_ELEVATION_STEP_COUNT_POINTS 1
#define COMPUTE_ELEVATION_STEP_CREATE_GROUND 2
#define COMPUTE_ELEVATION_STEP_CREATE_INDEX 3
#define COMPUTE_ELEVATION_STEP_COMPUTE_ELEVATION 4

ComputeElevationAction::ComputeElevationAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeElevationAction::~ComputeElevationAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeElevationAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    voxelRadius_ = 0;

    numberOfPoints_ = 0;
    numberOfGroundPoints_ = 0;
    numberOfNonGroundPoints_ = 0;
    pointIndex_ = 0;

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    points_.clear();
}

void ComputeElevationAction::start(double voxelRadius)
{
    LOG_DEBUG(<< "Start with voxelRadius <" << voxelRadius << ">.");

    // Set input parameters.
    double ppm = editor_->settings().units().pointsPerMeter()[0];

    voxelRadius_ = voxelRadius * ppm;

    // Clear work data.
    numberOfPoints_ = editor_->datasets().nPoints();
    numberOfGroundPoints_ = 0;
    numberOfNonGroundPoints_ = 0;
    pointIndex_ = 0;
    LOG_DEBUG(<< "Total number of points <" << numberOfPoints_ << ">.");

    elevationMinimum_ = 0;
    elevationMaximum_ = 0;

    zMinimum_ = editor_->boundary().min(2);
    LOG_DEBUG(<< "Z minimum_ <" << zMinimum_ << "> maximum <"
              << editor_->boundary().max(2) << ">.");

    points_.clear();

    // Plan the steps.
    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setMaximumSteps({14.0, 8.0, 35.0, 1.0, 42.0});
    progress_.setValueSteps(COMPUTE_ELEVATION_STEP_RESET_POINTS);
}

void ComputeElevationAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_ELEVATION_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_ELEVATION_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_ELEVATION_STEP_CREATE_GROUND:
            stepCreateGround();
            break;

        case COMPUTE_ELEVATION_STEP_CREATE_INDEX:
            stepCreateIndex();
            break;

        case COMPUTE_ELEVATION_STEP_COMPUTE_ELEVATION:
            stepComputeElevation();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeElevationAction::stepResetPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start step reset points.");

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
    // query_.flush();
    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setValueSteps(COMPUTE_ELEVATION_STEP_COUNT_POINTS);

    LOG_DEBUG(<< "Finished step reset points.");
}

void ComputeElevationAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start step count points.");

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

    LOG_DEBUG(<< "Number of ground points <" << numberOfGroundPoints_ << ">.");
    LOG_DEBUG(<< "Number of non-ground points <" << numberOfNonGroundPoints_
              << ">.");

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfGroundPoints_, 100);
    progress_.setValueSteps(COMPUTE_ELEVATION_STEP_CREATE_GROUND);

    LOG_DEBUG(<< "Finished step count points.");
}

void ComputeElevationAction::stepCreateGround()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start step create ground.");
    }

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
    progress_.setValueSteps(COMPUTE_ELEVATION_STEP_CREATE_INDEX);

    LOG_DEBUG(<< "Finished step create ground.");
}

void ComputeElevationAction::stepCreateIndex()
{
    LOG_DEBUG(<< "Start step create index.");

    // Create ground index.
    points_.createIndex();

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfNonGroundPoints_, 100);
    progress_.setValueSteps(COMPUTE_ELEVATION_STEP_COMPUTE_ELEVATION);

    LOG_DEBUG(<< "Finished step create index.");
}

void ComputeElevationAction::stepComputeElevation()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start step compute elevation.");
    }

    // Iterate all points:
    while (query_.next())
    {
        // If the current point is not classified as ground:
        if (query_.classification() != LasFile::CLASS_GROUND)
        {
            // Find nearest neighbour in ground projection:
            size_t idx = points_.findNN(query_.x(), query_.y(), zMinimum_);
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
                query_.setModified();
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

    LOG_DEBUG(<< "Finished step compute elevation.");
}

void ComputeElevationAction::createGroundPoint()
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
    p.z = zMinimum_;
    p.elevation = zMinimum_;

    // Compute point coordinates as average from all neighbour points:
    size_t n = 0;

    queryPoint_.where().setSphere(query_.x(),
                                  query_.y(),
                                  query_.z(),
                                  voxelRadius_);
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
