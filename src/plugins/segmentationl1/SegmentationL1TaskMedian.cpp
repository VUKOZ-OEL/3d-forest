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

/** @file SegmentationL1TaskMedian.cpp */

#include <Editor.hpp>
#include <SegmentationL1TaskMedian.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskMedian"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskMedian::initialize(SegmentationL1Context *context)
{
    context_ = context;

    context_->samples = context_->samplesBackup;
    context_->query.setWhere(context_->editor->viewports().where());

    index_ = 0;

    int numberOfIterations = context_->parameters.numberOfIterations;
    if (numberOfIterations < 1)
    {
        numberOfIterations = 1;
    }
    iterations_ = static_cast<size_t>(numberOfIterations);
    iteration_ = 0;
    setupSearchRadius();

    ProgressActionInterface::initialize(context_->samples.size() * iterations_);
}

void SegmentationL1TaskMedian::next()
{
    uint64_t n = process();
    uint64_t i = 0;

    startTimer();

    while (i < n)
    {
        step();

        i++;
        if (timedOut())
        {
            break;
        }
    }

    increment(i);
}

void SegmentationL1TaskMedian::step()
{
    if (index_ >= context_->samples.size())
    {
        // Start next iteration.
        index_ = 0;
        iteration_++;
        setupSearchRadius();
    }

    SegmentationL1Point &point = context_->samples[index_];

    double x = point.x;
    double y = point.y;
    double z = point.z;

    context_->query.where().setSphere(x, y, z, radius_);
    context_->query.exec();
    context_->query.mean(x, y, z);

    point.x = x;
    point.y = y;
    point.z = z;

    index_++;
}

void SegmentationL1TaskMedian::setupSearchRadius()
{
    double r1;
    double r2;

    r1 = static_cast<double>(context_->parameters.neighborhoodRadiusMinimum);
    r2 = static_cast<double>(context_->parameters.neighborhoodRadiusMaximum);

    if (iterations_ > 1)
    {
        double step = (r2 - r1) / static_cast<double>(iterations_ - 1);
        radius_ = r1 + (step * static_cast<double>(iteration_));
    }
    else
    {
        radius_ = r1;
    }
}
