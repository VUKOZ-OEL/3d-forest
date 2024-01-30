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

/** @file DescriptorAction.cpp */

// Include 3D Forest.
#include <DescriptorAction.hpp>
#include <Editor.hpp>
#include <Math.hpp>

// Include local.
#define LOG_MODULE_NAME "DescriptorAction"
#include <Log.hpp>

#define DESCRIPTOR_STEP_RESET_POINTS 0
#define DESCRIPTOR_STEP_COUNT_POINTS 1
#define DESCRIPTOR_STEP_COMPUTE 2
#define DESCRIPTOR_STEP_NORMALIZE 3

#define DESCRIPTOR_IGNORE 0
#define DESCRIPTOR_PROCESS 1
#define DESCRIPTOR_NOT_FOUND 2
#define DESCRIPTOR_FOUND 3

DescriptorAction::DescriptorAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor),
      pca_()
{
    LOG_DEBUG(<< "Create.");
}

DescriptorAction::~DescriptorAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void DescriptorAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    pca_.clear();

    radius_ = 0;
    voxelSize_ = 0;
    method_ = DescriptorAction::METHOD_DENSITY;
    includeGround_ = false;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    numberOfPoints_ = 0;
    numberOfPointsInFilter_ = 0;
    numberOfPointsWithDescriptor_ = 0;
}

void DescriptorAction::start(double radius,
                             double voxelSize,
                             Method method,
                             bool includeGround)
{
    LOG_DEBUG(<< "Start with parameter radius <" << radius << "> voxelSize <"
              << voxelSize << "> method <" << static_cast<int>(method) << ">.");

    radius_ = radius;
    voxelSize_ = voxelSize;
    method_ = method;
    includeGround_ = includeGround;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    numberOfPoints_ = editor_->datasets().nPoints();
    numberOfPointsInFilter_ = 0;
    numberOfPointsWithDescriptor_ = 0;

    progress_.setMaximumStep(numberOfPoints_, 1000);
    progress_.setMaximumSteps({5.0, 5.0, 85.0, 5.0});
    progress_.setValueSteps(DESCRIPTOR_STEP_RESET_POINTS);
}

void DescriptorAction::next()
{
    switch (progress_.valueSteps())
    {
        case DESCRIPTOR_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case DESCRIPTOR_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case DESCRIPTOR_STEP_COMPUTE:
            stepCompute();
            break;

        case DESCRIPTOR_STEP_NORMALIZE:
            stepNormalize();
            break;

        default:
            // Empty.
            break;
    }
}

void DescriptorAction::stepResetPoints()
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
        if (includeGround_ || query_.classification() != LasFile::CLASS_GROUND)
        {
            query_.voxel() = DESCRIPTOR_PROCESS;
        }
        else
        {
            query_.voxel() = DESCRIPTOR_IGNORE;
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
    progress_.setValueSteps(DESCRIPTOR_STEP_COUNT_POINTS);
}

void DescriptorAction::stepCountPoints()
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
    progress_.setValueSteps(DESCRIPTOR_STEP_COMPUTE);
}

void DescriptorAction::stepCompute()
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
    progress_.setValueSteps(DESCRIPTOR_STEP_NORMALIZE);
}

void DescriptorAction::stepNormalize()
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
            if (query_.voxel() == DESCRIPTOR_FOUND)
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

void DescriptorAction::computePoint()
{
    // Do nothing when this point is not marked for processing.
    if (query_.voxel() != DESCRIPTOR_PROCESS)
    {
        return;
    }

    // Compute descriptor value.
    double descriptor;
    bool hasDescriptor;

    if (method_ == DescriptorAction::METHOD_DENSITY)
    {
        queryPoint_.where().setSphere(query_.x(),
                                      query_.y(),
                                      query_.z(),
                                      radius_);
        queryPoint_.exec();

        descriptor = 0.0;
        hasDescriptor = true;
        while (queryPoint_.next())
        {
            if (query_.voxel() != DESCRIPTOR_IGNORE)
            {
                descriptor += 1.0;
            }
        }
    }
    else if (method_ == DescriptorAction::METHOD_PCA_INTENSITY)
    {
        double meanX;
        double meanY;
        double meanZ;

        hasDescriptor = pca_.computeDescriptor(queryPoint_,
                                               query_.x(),
                                               query_.y(),
                                               query_.z(),
                                               radius_,
                                               meanX,
                                               meanY,
                                               meanZ,
                                               descriptor);
    }
    else
    {
        hasDescriptor = false;
    }

    // Update descriptor minimum and maximum values.
    size_t newValue;
    if (hasDescriptor)
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
        newValue = DESCRIPTOR_FOUND;
    }
    else
    {
        newValue = DESCRIPTOR_NOT_FOUND;
    }

    // Distribute computed descriptor value to neighbors.
    if (voxelSize_ > 1.0)
    {
        queryPoint_.where().setSphere(query_.x(),
                                      query_.y(),
                                      query_.z(),
                                      voxelSize_);
        queryPoint_.exec();

        while (queryPoint_.next())
        {
            size_t oldValue = queryPoint_.voxel();
            if ((oldValue == DESCRIPTOR_PROCESS) ||
                (oldValue == DESCRIPTOR_NOT_FOUND &&
                 newValue == DESCRIPTOR_FOUND))
            {
                queryPoint_.voxel() = newValue;
                if (newValue == DESCRIPTOR_FOUND)
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
        if (newValue == DESCRIPTOR_FOUND)
        {
            query_.descriptor() = descriptor;
        }
        query_.setModified();
    }
}
