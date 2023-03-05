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
#include <Geometry.hpp>
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

    uint64_t n = context_->samples.size() * iterations_;
    LOG_DEBUG(<< "n <" << n << ">.");

    ProgressActionInterface::initialize(n);
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
    index_++;

    // Setup slice parameters.
    Vector3<double> u(point.x, point.y, point.z);
    Vector3<double> v(point.x, point.y, point.z);
    Vector3<double> d(point.vx, point.vy, point.vz);

    double r = radius_;
    double h = radius_ * 0.25;
    Vector3<double> a = u - (d * h);
    Vector3<double> b = u + (d * h);

    // Select slice boundary.
    context_->query.where().setCylinder(a[0], a[1], a[2], b[0], b[1], b[2], r);
    context_->query.exec();

    // Compute L1 median.
    context_->median.median(context_->query, v[0], v[1], v[2]);

    point.x = v[0];
    point.y = v[1];
    point.z = v[2];
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
