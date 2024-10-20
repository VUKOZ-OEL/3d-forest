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

/** @file ComputeClassificationAction.cpp */

// Include 3D Forest.
#include <ComputeClassificationAction.hpp>
#include <Cone.hpp>
#include <Editor.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeClassificationAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_CLASSIFICATION_STEP_RESET_POINTS 0
#define COMPUTE_CLASSIFICATION_STEP_COUNT_POINTS 1
#define COMPUTE_CLASSIFICATION_STEP_POINTS_TO_VOXELS 2
#define COMPUTE_CLASSIFICATION_STEP_CREATE_VOXEL_INDEX 3
#define COMPUTE_CLASSIFICATION_STEP_CLASSIFY_GROUND 4
#define COMPUTE_CLASSIFICATION_STEP_VOXELS_TO_POINTS 5

#define COMPUTE_CLASSIFICATION_PROCESS 0
#define COMPUTE_CLASSIFICATION_NOT_FOUND 1
#define COMPUTE_CLASSIFICATION_FOUND 2

ComputeClassificationAction::ComputeClassificationAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeClassificationAction::~ComputeClassificationAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeClassificationAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    nPointsTotal_ = 0;
    nPointsInFilter_ = 0;

    voxels_.clear();
    group_.clear();
    path_.clear();
    searchNext_.clear();
    searchGround_.clear();

    minimumIndex_ = 0;
    minimumValue_ = 0;
}

void ComputeClassificationAction::start(
    const ComputeClassificationParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().units().pointsPerMeter()[0];
    LOG_DEBUG(<< "Units pointsPerMeter <" << ppm << ">.");

    parameters_ = parameters;

    parameters_.voxelRadius *= ppm;
    parameters_.searchRadius *= ppm;

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;

    voxels_.clear();
    group_.clear();
    path_.clear();
    searchNext_.clear();
    searchGround_.clear();

    minimumIndex_ = SIZE_MAX;
    minimumValue_ = std::numeric_limits<double>::max();

    // Plan the steps.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps({20.0, 10.0, 20.0, 20.0, 10.0, 20.0});
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_RESET_POINTS);
}

void ComputeClassificationAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_CLASSIFICATION_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_CLASSIFICATION_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_CLASSIFICATION_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case COMPUTE_CLASSIFICATION_STEP_CREATE_VOXEL_INDEX:
            stepCreateVoxelIndex();
            break;

        case COMPUTE_CLASSIFICATION_STEP_CLASSIFY_GROUND:
            stepClassifyGround();
            break;

        case COMPUTE_CLASSIFICATION_STEP_VOXELS_TO_POINTS:
            stepVoxelsToPoints();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeClassificationAction::stepResetPoints()
{
    progress_.startTimer();

    // Initialize:
    if (progress_.valueStep() == 0)
    {
        // Reset elevation range.
        Range<double> range;
        editor_->setElevationFilter(range);

        // Set query to iterate all points. The active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // For each point in all datasets:
    while (query_.next())
    {
        // Set point index to voxel to none.
        query_.voxel() = SIZE_MAX;

        // Reset point classification of ground points to never classified.
        if (parameters_.cleanAllClassifications ||
            (parameters_.cleanGroundClassifications &&
             query_.classification() == LasFile::CLASS_GROUND))
        {
            query_.classification() = LasFile::CLASS_NEVER_CLASSIFIED;
        }

        // Reset point elevation to zero.
        query_.elevation() = 0;

        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_COUNT_POINTS);
}

void ComputeClassificationAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize:
    if (progress_.valueStep() == 0)
    {
        // Set query to use the active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    // Count the number of filtered points.
    // This can be the same value as nPointsTotal.
    while (query_.next())
    {
        nPointsInFilter_++;

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Counted <" << nPointsInFilter_ << "> points.");

    // Next.
    query_.reset();
    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_POINTS_TO_VOXELS);
}

void ComputeClassificationAction::stepPointsToVoxels()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point index to voxel is none:
        if (query_.voxel() == SIZE_MAX)
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

    LOG_DEBUG(<< "Created <" << voxels_.size() << "> voxels.");

    // Next.
    query_.reset();
    progress_.setMaximumStep(voxels_.size(), 100);
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_CREATE_VOXEL_INDEX);
}

void ComputeClassificationAction::stepCreateVoxelIndex()
{
    // Create voxel index.
    voxels_.createIndex();

    LOG_DEBUG(<< "Created index.");

    // Next.
    progress_.setMaximumStep(voxels_.size(), 10);
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_CLASSIFY_GROUND);
}

void ComputeClassificationAction::stepClassifyGround()
{
    progress_.startTimer();

    // Initialize:
    if (progress_.valueStep() == 0)
    {
        // If there is at least one voxel:
        if (minimumIndex_ < voxels_.size())
        {
            // Append this voxel into ground path.
            voxels_[minimumIndex_].group = COMPUTE_CLASSIFICATION_FOUND;
            path_.push_back(minimumIndex_);
        }

        progress_.addValueStep(1);
    }

    // While the path is not empty:
    while (path_.size() > 0)
    {
        // Add the path to the current group.
        size_t idx = group_.size();
        for (size_t i = 0; i < path_.size(); i++)
        {
            group_.push_back(path_[i]);
        }

        // Set the path empty.
        path_.resize(0);

        // Try to expand the current group with neighbor voxels.
        for (size_t i = idx; i < group_.size(); i++)
        {
            Point &a = voxels_[group_[i]];
            progress_.addValueStep(1);

            voxels_.findRadius(a.x,
                               a.y,
                               a.z,
                               parameters_.searchRadius,
                               searchNext_);
            for (size_t j = 0; j < searchNext_.size(); j++)
            {
                // If a neighbor voxel is not yet processed:
                Point &b = voxels_[searchNext_[j]];
                if (b.group == COMPUTE_CLASSIFICATION_PROCESS)
                {
                    bool ground = true;

                    // Select cone below this neighbor voxel.
                    Cone<double> cone;
                    cone.set(b.x,
                             b.y,
                             b.z,
                             minimumValue_,
                             90.0 - parameters_.angle);
                    Vector3<double> p = cone.box().center();
                    double r = cone.box().radius();

                    voxels_.findRadius(p[0], p[1], p[2], r, searchGround_);
                    for (size_t k = 0; k < searchGround_.size(); k++)
                    {
                        Point &c = voxels_[searchGround_[k]];
                        if (cone.contains(c.x, c.y, c.z))
                        {
                            ground = false;
                            break;
                        }
                    }

                    // If the selection cone is empty:
                    if (ground)
                    {
                        // Mark this neighbor voxel as ground.
                        b.group = COMPUTE_CLASSIFICATION_FOUND;

                        // Append this voxel into path.
                        path_.push_back(searchNext_[j]);
                    }
                    // Otherwise, mark this voxel as processed.
                    else
                    {
                        b.group = COMPUTE_CLASSIFICATION_NOT_FOUND;
                    }
                }
            }
        }
    }

    // Next.
    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_CLASSIFICATION_STEP_VOXELS_TO_POINTS);
}

void ComputeClassificationAction::stepVoxelsToPoints()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If a point belongs to some voxel:
        size_t pointIndex = query_.voxel();
        if (pointIndex < voxels_.size())
        {
            // If this voxel is marked as ground:
            if (voxels_[pointIndex].group == COMPUTE_CLASSIFICATION_FOUND)
            {
                // Set classification of this point as ground.
                query_.classification() = LasFile::CLASS_GROUND;
                query_.setModified();
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Done.");

    // Flush all modifications.
    query_.flush();

    // All steps are now complete.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void ComputeClassificationAction::createVoxel()
{
    // Mark index of new voxel in voxel array.
    size_t idx = voxels_.size();

    // Initialize new voxel point.
    Point p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    p.group = COMPUTE_CLASSIFICATION_PROCESS;

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
        p.x += queryPoint_.x();
        p.y += queryPoint_.y();
        p.z += queryPoint_.z();

        queryPoint_.voxel() = idx;
        queryPoint_.setModified();

        n++;
    }

    if (n < 1)
    {
        return;
    }

    p.x = p.x / static_cast<double>(n);
    p.y = p.y / static_cast<double>(n);
    p.z = p.z / static_cast<double>(n);

    // Update minimum height.
    if (p.z < minimumValue_)
    {
        minimumIndex_ = idx;
        minimumValue_ = p.z;
    }

    // Append new voxel to voxel array.
    voxels_.push_back(std::move(p));
}
