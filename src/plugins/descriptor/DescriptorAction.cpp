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

#include <DescriptorAction.hpp>
#include <Editor.hpp>
#include <Math.hpp>

#define LOG_MODULE_NAME "DescriptorAction"
#include <Log.hpp>

#define DESCRIPTOR_STEP_COUNT 0
#define DESCRIPTOR_STEP_CLEAR 1
#define DESCRIPTOR_STEP_COMPUTE 2
#define DESCRIPTOR_STEP_NORMALIZE 3

DescriptorAction::DescriptorAction(Editor *editor)
    : editor_(editor),
      queryPoints_(editor),
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

    queryPoints_.clear();
    queryPoint_.clear();

    pca_.clear();

    radius_ = 0;
    voxelSize_ = 0;
    method_ = DescriptorAction::METHOD_DENSITY;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    nPointsTotal_ = 0;
    nPointsWithDescriptor_ = 0;
}

void DescriptorAction::start(double radius, double voxelSize, Method method)
{
    LOG_DEBUG(<< "Start with parameter radius <" << radius << "> voxelSize <"
              << voxelSize << "> method <" << static_cast<int>(method) << ">.");

    radius_ = radius;
    voxelSize_ = voxelSize;
    method_ = method;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    nPointsTotal_ = 0;
    nPointsWithDescriptor_ = 0;

    queryPoints_.setWhere(editor_->viewports().where());
    queryPoints_.exec();

    progress_.setMaximumStep(ProgressCounter::npos, 1000UL);
    progress_.setMaximumSteps({1.0, 2.0, 93.0, 4.0});
    progress_.setValueSteps(DESCRIPTOR_STEP_COUNT);
}

void DescriptorAction::next()
{
    if (progress_.valueSteps() == DESCRIPTOR_STEP_COUNT)
    {
        stepCount();
    }
    else if (progress_.valueSteps() == DESCRIPTOR_STEP_CLEAR)
    {
        stepClear();
    }
    else if (progress_.valueSteps() == DESCRIPTOR_STEP_COMPUTE)
    {
        stepCompute();
    }
    else if (progress_.valueSteps() == DESCRIPTOR_STEP_NORMALIZE)
    {
        stepNormalize();
    }
}

void DescriptorAction::stepCount()
{
    progress_.startTimer();

    while (queryPoints_.next())
    {
        nPointsTotal_++;

        if (progress_.timedOut())
        {
            return;
        }
    }

    queryPoints_.reset();

    progress_.setMaximumStep(nPointsTotal_, 1000U);
    progress_.setValueSteps(DESCRIPTOR_STEP_CLEAR);
}

void DescriptorAction::stepClear()
{
    progress_.startTimer();

    while (queryPoints_.next())
    {
        queryPoints_.value() = 0;
        queryPoints_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    queryPoints_.reset();

    progress_.setMaximumStep(nPointsTotal_, 25U);
    progress_.setValueSteps(DESCRIPTOR_STEP_COMPUTE);
}

void DescriptorAction::stepCompute()
{
    progress_.startTimer();

    while (queryPoints_.next())
    {
        if (queryPoints_.value() == 0)
        {
            computePoint();
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    queryPoints_.reset();

    progress_.setMaximumStep(nPointsTotal_, 1000U);
    progress_.setValueSteps(DESCRIPTOR_STEP_NORMALIZE);
}

void DescriptorAction::computePoint()
{
    double descriptor;
    bool hasDescriptor;

    if (method_ == DescriptorAction::METHOD_DENSITY)
    {
        queryPoint_.where().setSphere(queryPoints_.x(),
                                      queryPoints_.y(),
                                      queryPoints_.z(),
                                      radius_);
        queryPoint_.exec();

        descriptor = 0.0;
        hasDescriptor = true;
        while (queryPoint_.next())
        {
            descriptor += 1.0;
        }
    }
    else if (method_ == DescriptorAction::METHOD_PCA)
    {
        double meanX;
        double meanY;
        double meanZ;

        hasDescriptor = pca_.computeDescriptor(queryPoint_,
                                               queryPoints_.x(),
                                               queryPoints_.y(),
                                               queryPoints_.z(),
                                               radius_,
                                               meanX,
                                               meanY,
                                               meanZ,
                                               descriptor);
    }
    else
    {
        hasDescriptor = pca_.computeDistribution(queryPoint_,
                                                 queryPoints_.x(),
                                                 queryPoints_.y(),
                                                 queryPoints_.z(),
                                                 radius_,
                                                 descriptor);
    }

    if (hasDescriptor)
    {
        if (nPointsWithDescriptor_ == 0)
        {
            descriptorMinimum_ = descriptor;
            descriptorMaximum_ = descriptor;
        }
        else
        {
            updateRange(descriptor, descriptorMinimum_, descriptorMaximum_);
        }

        nPointsWithDescriptor_++;
    }
    else
    {
        descriptor = std::numeric_limits<double>::max();
    }

    queryPoints_.value() = 1;
    queryPoints_.descriptor() = descriptor;

    if (voxelSize_ > 1.0)
    {
        queryPoint_.where().setSphere(queryPoints_.x(),
                                      queryPoints_.y(),
                                      queryPoints_.z(),
                                      voxelSize_);
        queryPoint_.exec();

        while (queryPoint_.next())
        {
            queryPoint_.value() = 1;
            queryPoint_.descriptor() = descriptor;
            queryPoint_.setModified();
        }
    }

    queryPoints_.setModified();
}

void DescriptorAction::stepNormalize()
{
    progress_.startTimer();

    double descriptorRange_ = descriptorMaximum_ - descriptorMinimum_;

    if (descriptorRange_ > 0.0)
    {
        double d = 1.0 / descriptorRange_;

        while (queryPoints_.next())
        {
            double descriptor = queryPoints_.descriptor();
            if (descriptor < std::numeric_limits<double>::max())
            {
                descriptor = (descriptor - descriptorMinimum_) * d;
            }
            else
            {
                descriptor = 0;
            }

            queryPoints_.descriptor() = descriptor;
            queryPoints_.setModified();

            progress_.addValueStep(1);
            if (progress_.timedOut())
            {
                return;
            }
        }
    }
    else
    {
        while (queryPoints_.next())
        {
            queryPoints_.descriptor() = 0;
            queryPoints_.setModified();

            progress_.addValueStep(1);
            if (progress_.timedOut())
            {
                return;
            }
        }
    }

    queryPoints_.flush();

    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}
