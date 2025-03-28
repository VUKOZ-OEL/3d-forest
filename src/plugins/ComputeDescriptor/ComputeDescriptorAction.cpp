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

/** @file ComputeDescriptorAction.cpp */

// Include 3D Forest.
#include <ComputeDescriptorAction.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeDescriptorAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_DESCRIPTOR_STEP_RESET_POINTS 0
#define COMPUTE_DESCRIPTOR_STEP_COUNT_POINTS 1
#define COMPUTE_DESCRIPTOR_STEP_COMPUTE 2
#define COMPUTE_DESCRIPTOR_STEP_NORMALIZE 3

#define COMPUTE_DESCRIPTOR_IGNORE 0
#define COMPUTE_DESCRIPTOR_PROCESS 1
#define COMPUTE_DESCRIPTOR_NOT_FOUND 2
#define COMPUTE_DESCRIPTOR_FOUND 3

ComputeDescriptorAction::ComputeDescriptorAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor),
      pca_()
{
    LOG_DEBUG(<< "Create.");
}

ComputeDescriptorAction::~ComputeDescriptorAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeDescriptorAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    pca_.clear();

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    numberOfPoints_ = 0;
    numberOfPointsInFilter_ = 0;
    numberOfPointsWithDescriptor_ = 0;
}

void ComputeDescriptorAction::start(
    const ComputeDescriptorParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().unitsSettings().pointsPerMeter()[0];
    LOG_DEBUG(<< "Units pointsPerMeter <" << ppm << ">.");

    parameters_ = parameters;

    parameters_.voxelRadius *= ppm;
    parameters_.searchRadius *= ppm;

    // Clear work data.
    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    numberOfPoints_ = editor_->datasets().nPoints();
    numberOfPointsInFilter_ = 0;
    numberOfPointsWithDescriptor_ = 0;

    // Plan the steps.
    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setMaximumSteps({5.0, 5.0, 85.0, 5.0});
    progress_.setValueSteps(COMPUTE_DESCRIPTOR_STEP_RESET_POINTS);
}

void ComputeDescriptorAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_DESCRIPTOR_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_DESCRIPTOR_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_DESCRIPTOR_STEP_COMPUTE:
            stepCompute();
            break;

        case COMPUTE_DESCRIPTOR_STEP_NORMALIZE:
            stepNormalize();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeDescriptorAction::stepResetPoints()
{
    progress_.startTimer();

    // Initialize:
    if (progress_.valueStep() == 0)
    {
        // Set query to iterate all points. The active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // Clear each point in all datasets.
    while (query_.next())
    {
        if (parameters_.includeGroundPoints ||
            query_.classification() != LasFile::CLASS_GROUND)
        {
            query_.voxel() = COMPUTE_DESCRIPTOR_PROCESS;
        }
        else
        {
            query_.voxel() = COMPUTE_DESCRIPTOR_IGNORE;
        }

        query_.descriptor() = 0;
        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setValueSteps(COMPUTE_DESCRIPTOR_STEP_COUNT_POINTS);
}

void ComputeDescriptorAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize:
    if (progress_.valueStep() == 0)
    {
        // Set query to use the active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    // Iterate all filtered points.
    while (query_.next())
    {
        numberOfPointsInFilter_++;

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfPointsInFilter_, 25);
    progress_.setValueSteps(COMPUTE_DESCRIPTOR_STEP_COMPUTE);
}

void ComputeDescriptorAction::stepCompute()
{
    progress_.startTimer();

    // Iterate and compute all filtered points.
    while (query_.next())
    {
        computePoint();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    // Next.
    query_.reset();
    progress_.setMaximumStep(numberOfPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_DESCRIPTOR_STEP_NORMALIZE);
}

void ComputeDescriptorAction::stepNormalize()
{
    progress_.startTimer();

    // If descriptor range is greater than zero:
    double descriptorRange_ = descriptorMaximum_ - descriptorMinimum_;
    if (descriptorRange_ > 0.0)
    {
        double d = 1.0 / descriptorRange_;

        // Iterate all filtered points:
        while (query_.next())
        {
            // If a point is in state finished, then normalize its descriptor.
            if (query_.voxel() == COMPUTE_DESCRIPTOR_FOUND)
            {
                double descriptor = query_.descriptor();
                descriptor = (descriptor - descriptorMinimum_) * d;
                query_.descriptor() = descriptor;
                query_.setModified();
            }

            progress_.addValueStep(1);
            if (progress_.timedOut())
            {
                return;
            }
        }
    }

    // Flush all modifications.
    query_.flush();

    // All steps are now complete.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void ComputeDescriptorAction::computePoint()
{
    // Do nothing when this point is not marked for processing.
    if (query_.voxel() != COMPUTE_DESCRIPTOR_PROCESS)
    {
        return;
    }

    // Compute descriptor value.
    double descriptor;
    bool descriptorCalculated;

    if (parameters_.method == ComputeDescriptorParameters::METHOD_DENSITY)
    {
        queryPoint_.where().setSphere(query_.x(),
                                      query_.y(),
                                      query_.z(),
                                      parameters_.searchRadius);
        queryPoint_.exec();

        descriptor = 0.0;
        descriptorCalculated = true;
        while (queryPoint_.next())
        {
            if (query_.voxel() != COMPUTE_DESCRIPTOR_IGNORE)
            {
                descriptor += 1.0;
            }
        }
    }
    else if (parameters_.method ==
             ComputeDescriptorParameters::METHOD_PCA_INTENSITY)
    {
        double meanX;
        double meanY;
        double meanZ;

        descriptorCalculated = pca_.computeDescriptor(queryPoint_,
                                                      query_.x(),
                                                      query_.y(),
                                                      query_.z(),
                                                      parameters_.searchRadius,
                                                      meanX,
                                                      meanY,
                                                      meanZ,
                                                      descriptor);
    }
    else
    {
        descriptorCalculated = false;
    }

    // Update descriptor minimum and maximum values.
    size_t newValue;
    if (descriptorCalculated)
    {
        if (numberOfPointsWithDescriptor_ == 0)
        {
            descriptorMinimum_ = descriptor;
            descriptorMaximum_ = descriptor;
        }
        else
        {
            updateRange(descriptor, descriptorMinimum_, descriptorMaximum_);
        }

        numberOfPointsWithDescriptor_++;
        newValue = COMPUTE_DESCRIPTOR_FOUND;
    }
    else
    {
        newValue = COMPUTE_DESCRIPTOR_NOT_FOUND;
    }

    // Distribute computed descriptor value to neighbors.
    if (parameters_.voxelRadius > 1.0)
    {
        queryPoint_.where().setSphere(query_.x(),
                                      query_.y(),
                                      query_.z(),
                                      parameters_.voxelRadius);
        queryPoint_.exec();

        while (queryPoint_.next())
        {
            size_t oldValue = queryPoint_.voxel();
            if ((oldValue == COMPUTE_DESCRIPTOR_PROCESS) ||
                (oldValue == COMPUTE_DESCRIPTOR_NOT_FOUND &&
                 newValue == COMPUTE_DESCRIPTOR_FOUND))
            {
                queryPoint_.voxel() = newValue;
                if (newValue == COMPUTE_DESCRIPTOR_FOUND)
                {
                    queryPoint_.descriptor() = descriptor;
                }
                queryPoint_.setModified();
            }
        }
    }
    else
    {
        query_.voxel() = newValue;
        if (newValue == COMPUTE_DESCRIPTOR_FOUND)
        {
            query_.descriptor() = descriptor;
        }
        query_.setModified();
    }
}
