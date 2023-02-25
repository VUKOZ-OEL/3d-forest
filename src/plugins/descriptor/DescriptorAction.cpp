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

DescriptorAction::DescriptorAction(Editor *editor)
    : editor_(editor),
      queryPoints_(editor),
      queryPoint_(editor),
      pca_(),
      status_(STATUS_NEW)
{
    LOG_DEBUG(<< "Called.");
}

DescriptorAction::~DescriptorAction()
{
    LOG_DEBUG(<< "Called.");
}

void DescriptorAction::clear()
{
    LOG_DEBUG(<< "Called.");

    queryPoints_.clear();
    queryPoint_.clear();

    pca_.clear();

    status_ = STATUS_NEW;

    radius_ = 0;

    nPointsTotal_ = 0;
    nPointsOneHalf_ = 0;
    nPointsDone_ = 0;
    nPointsWithDescriptor_ = 0;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;
}

void DescriptorAction::initialize(double radius,
                                  double voxelSize,
                                  Method method)
{
    LOG_DEBUG(<< "Called with parameter radius <" << radius << ">.");

    radius_ = radius;
    voxelSize_ = voxelSize;
    method_ = method;

    status_ = STATUS_COMPUTE_DESCRIPTOR;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    nPointsTotal_ = 0;
    nPointsOneHalf_ = 0;
    nPointsDone_ = 0;
    nPointsWithDescriptor_ = 0;

    queryPoints_.setWhere(editor_->viewports().where());
    queryPoints_.exec();
    if (queryPoints_.next())
    {
        nPointsTotal_++;
    }

    ProgressActionInterface::initialize(ProgressActionInterface::npos, 1000UL);
}

void DescriptorAction::next()
{
    if (initializing())
    {
        determineMaximum();
    }
    else if (status_ == STATUS_COMPUTE_DESCRIPTOR)
    {
        stepComputeDescriptor();
    }
    else if (status_ == STATUS_NORMALIZE_DESCRIPTOR)
    {
        stepNormalizeDescriptor();
    }
}

void DescriptorAction::determineMaximum()
{
    startTimer();

    while (queryPoints_.next())
    {
        nPointsTotal_++;

        if (timedOut())
        {
            return;
        }
    }

    queryPoints_.reset();

    nPointsOneHalf_ = nPointsTotal_;
    nPointsTotal_ *= 2U;

    ProgressActionInterface::initialize(nPointsTotal_, 25UL);
}

void DescriptorAction::stepComputeDescriptor()
{
    uint64_t i = 0;

    startTimer();

    while (nPointsDone_ < nPointsOneHalf_)
    {
        if (queryPoints_.next())
        {
            double descriptor;
            bool hasDescriptor;

            if (method_ == DescriptorAction::METHOD_PCA)
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
                queryPoints_.value() = static_cast<size_t>(descriptor * 1e5);
                queryPoints_.setModified();

                if (nPointsWithDescriptor_ == 0)
                {
                    descriptorMinimum_ = descriptor;
                    descriptorMaximum_ = descriptor;
                }
                else
                {
                    updateRange(descriptor,
                                descriptorMinimum_,
                                descriptorMaximum_);
                }

                nPointsWithDescriptor_++;
            }
        }

        i++;
        nPointsDone_++;

        if (timedOut())
        {
            break;
        }
    }

    increment(i);

    if (nPointsDone_ == nPointsOneHalf_)
    {
        LOG_DEBUG(<< "Change state to STATUS_NORMALIZE_DESCRIPTOR.");
        status_ = STATUS_NORMALIZE_DESCRIPTOR;
        queryPoints_.reset();
    }
}

void DescriptorAction::stepNormalizeDescriptor()
{
    uint64_t i = 0;

    startTimer();

    double descriptorRange_ = descriptorMaximum_ - descriptorMinimum_;

    if (descriptorRange_ > 0.0)
    {
        double d = 1.0 / descriptorRange_;

        while (nPointsDone_ < nPointsTotal_)
        {
            if (queryPoints_.next())
            {
                double v = static_cast<double>(queryPoints_.value()) * 1e-5;
                queryPoints_.descriptor() = (v - descriptorMinimum_) * d;
                queryPoints_.setModified();
            }

            i++;
            nPointsDone_++;
            if (timedOut())
            {
                break;
            }
        }
    }
    else
    {
        while (nPointsDone_ < nPointsTotal_)
        {
            if (queryPoints_.next())
            {
                queryPoints_.descriptor() = 0;
                queryPoints_.setModified();
            }

            i++;
            nPointsDone_++;
            if (timedOut())
            {
                break;
            }
        }
    }

    increment(i);

    if (nPointsDone_ == nPointsTotal_)
    {
        LOG_DEBUG(<< "Change state to STATUS_FINISHED.");
        status_ = STATUS_FINISHED;
        queryPoints_.flush();
    }
}
