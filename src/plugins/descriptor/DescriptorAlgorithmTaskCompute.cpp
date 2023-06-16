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

/** @file DescriptorAlgorithmTaskCompute.cpp */

#include <DescriptorAlgorithmTaskCompute.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "DescriptorAlgorithmTaskCompute"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void DescriptorAlgorithmTaskCompute::initialize(
    DescriptorAlgorithmContext *context)
{
    context_ = context;
    context_->descriptorMinimum = 0;
    context_->descriptorMaximum = 0;
    context_->query.reset();
    nPointsWithDescriptor_ = 0;

    ProgressActionInterface::initialize(context_->nPoints);
}

void DescriptorAlgorithmTaskCompute::next()
{
    startTimer();

    while (context_->query.next())
    {
        if (context_->parameters.method ==
            DescriptorAlgorithmParameters::METHOD_PCA)
        {
            stepPca();
        }
        else
        {
            stepDensity();
        }

        increment(1);

        if (timedOut())
        {
            return;
        }
    }

    setProcessed(maximum());
}

void DescriptorAlgorithmTaskCompute::stepPca()
{
    double r = static_cast<double>(context_->parameters.neighborhoodRadius);

    bool hasDescriptor;
    double descriptor;
    double meanX;
    double meanY;
    double meanZ;

    hasDescriptor = pca_.computeDescriptor(context_->queryPoint,
                                           context_->query.x(),
                                           context_->query.y(),
                                           context_->query.z(),
                                           r,
                                           meanX,
                                           meanY,
                                           meanZ,
                                           descriptor);

    if (hasDescriptor)
    {
        context_->query.descriptor() = descriptor;

        if (nPointsWithDescriptor_ == 0)
        {
            context_->descriptorMinimum = descriptor;
            context_->descriptorMaximum = descriptor;
        }
        else
        {
            updateRange(descriptor,
                        context_->descriptorMinimum,
                        context_->descriptorMaximum);
        }

        nPointsWithDescriptor_++;
    }
    else
    {
        context_->query.descriptor() = std::numeric_limits<double>::max();
    }

    context_->query.setModified();
}

void DescriptorAlgorithmTaskCompute::stepDensity()
{
    double r = static_cast<double>(context_->parameters.neighborhoodRadius);

    context_->queryPoint.where().setSphere(context_->query.x(),
                                           context_->query.y(),
                                           context_->query.z(),
                                           r);
    context_->queryPoint.exec();

    size_t nPointsSphere = 0;
    while (context_->queryPoint.next())
    {
        nPointsSphere++;
    }

    double descriptor = static_cast<double>(nPointsSphere);

    context_->query.descriptor() = descriptor;
    context_->query.setModified();

    if (processed() == 0)
    {
        context_->descriptorMinimum = descriptor;
        context_->descriptorMaximum = descriptor;
    }
    else
    {
        updateRange(descriptor,
                    context_->descriptorMinimum,
                    context_->descriptorMaximum);
    }
}
