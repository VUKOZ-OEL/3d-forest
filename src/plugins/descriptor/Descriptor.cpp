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

/** @file Descriptor.cpp */

#include <Descriptor.hpp>
#include <Editor.hpp>
#include <Math.hpp>

#define MODULE_NAME "Descriptor"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

Descriptor::Descriptor(Editor *editor)
    : editor_(editor),
      queryPoints_(editor),
      queryPoint_(editor),
      pca_(),
      status_(STATUS_NEW)
{
    LOG_DEBUG_LOCAL("");
}

Descriptor::~Descriptor()
{
    LOG_DEBUG_LOCAL("");
}

void Descriptor::clear()
{
    LOG_DEBUG_LOCAL("");

    queryPoints_.clear();
    queryPoint_.clear();

    pca_.clear();

    status_ = STATUS_NEW;

    radius_ = 0;

    currentStep_ = 0;
    numberOfSteps_ = 0;

    nPointsTotal_ = 0;
    nPointsPerStep_ = 0;
    nPointsProcessed_ = 0;
    nPointsWithDescriptor_ = 0;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;
}

int Descriptor::start(double radius, double voxelSize, Method method)
{
    LOG_DEBUG_LOCAL("radius <" << radius << ">");

    radius_ = radius;
    voxelSize_ = voxelSize;
    method_ = method;

    status_ = STATUS_COMPUTE_DESCRIPTOR;

    descriptorMinimum_ = 0;
    descriptorMaximum_ = 0;

    nPointsTotal_ = 0;
    queryPoints_.where().setBox(editor_->clipBoundary());
    queryPoints_.exec();
    while (queryPoints_.next())
    {
        nPointsTotal_++;
    }

    nPointsPerStep_ = 1000;
    nPointsProcessed_ = 0;
    nPointsWithDescriptor_ = 0;

    uint64_t nSteps = nPointsTotal_ / nPointsPerStep_;
    if (nPointsTotal_ % nPointsPerStep_ > 0)
    {
        nSteps++;
    }

    currentStep_ = 0;
    numberOfSteps_ = static_cast<int>(nSteps * 2);

    LOG_DEBUG_LOCAL("numberOfSteps <" << numberOfSteps_ << ">");

    return numberOfSteps_;
}

void Descriptor::step()
{
    LOG_DEBUG_LOCAL("step <" << (currentStep_ + 1) << "> from <"
                             << numberOfSteps_ << ">");

    if (status_ == STATUS_COMPUTE_DESCRIPTOR)
    {
        stepComputeDescriptor();
        currentStep_++;
    }
    else if (status_ == STATUS_NORMALIZE_DESCRIPTOR)
    {
        stepNormalizeDescriptor();
        currentStep_++;
    }
}

void Descriptor::stepComputeDescriptor()
{
    uint64_t nPointsToProcess = nPointsTotal_ - nPointsProcessed_;
    if (nPointsToProcess > nPointsPerStep_)
    {
        nPointsToProcess = nPointsPerStep_;
    }
    LOG_DEBUG_LOCAL("points to process <" << nPointsToProcess << "> start <"
                                          << nPointsProcessed_ << "> total <"
                                          << nPointsTotal_ << ">");

    if (nPointsProcessed_ == 0)
    {
        queryPoints_.reset();
    }

    uint64_t i = 0;

    while (i < nPointsToProcess && queryPoints_.next())
    {
        float descriptor;
        bool hasDescriptor;

        if (method_ == Descriptor::METHOD_PCA)
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
            queryPoints_.value() = static_cast<size_t>(descriptor * 1e5F);
            queryPoints_.setModified();

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

        i++;
    }

    nPointsProcessed_ += nPointsToProcess;
    LOG_DEBUG_LOCAL("points processed <" << nPointsProcessed_ << "> from <"
                                         << nPointsTotal_ << ">");

    if (nPointsProcessed_ == nPointsTotal_)
    {
        status_ = STATUS_NORMALIZE_DESCRIPTOR;
        nPointsProcessed_ = 0;
    }
}

void Descriptor::stepNormalizeDescriptor()
{
    uint64_t nPointsToProcess = nPointsTotal_ - nPointsProcessed_;
    if (nPointsToProcess > nPointsPerStep_)
    {
        nPointsToProcess = nPointsPerStep_;
    }
    LOG_DEBUG_LOCAL("points to process <" << nPointsToProcess << "> start <"
                                          << nPointsProcessed_ << "> total <"
                                          << nPointsTotal_ << ">");

    if (nPointsProcessed_ == 0)
    {
        queryPoints_.reset();
    }

    float descriptorRange_ = descriptorMaximum_ - descriptorMinimum_;

    uint64_t i = 0;

    if (descriptorRange_ > 0.0F)
    {
        float d = 1.0F / descriptorRange_;

        while (i < nPointsToProcess && queryPoints_.next())
        {
            float v = static_cast<float>(queryPoints_.value()) * 1e-5F;
            queryPoints_.descriptor() = (v - descriptorMinimum_) * d;
            queryPoints_.setModified();
            i++;
        }
    }
    else
    {
        while (i < nPointsToProcess && queryPoints_.next())
        {
            queryPoints_.descriptor() = 0;
            queryPoints_.setModified();
            i++;
        }
    }

    nPointsProcessed_ += nPointsToProcess;
    LOG_DEBUG_LOCAL("points processed <" << nPointsProcessed_ << "> from <"
                                         << nPointsTotal_ << ">");

    if (nPointsProcessed_ == nPointsTotal_)
    {
        status_ = STATUS_FINISHED;
        nPointsProcessed_ = 0;
        queryPoints_.flush();
    }
}
