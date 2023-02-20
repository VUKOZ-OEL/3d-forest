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
    index_++;

    // Setup slice parameters.
    double d;
    double m = radius_ * 0.5;
    double r = std::sqrt((radius_ * radius_) + (radius_ * radius_));

    // Select slice boundary.
    context_->query.where().setSphere(point.x, point.y, point.z, r);
    context_->query.exec();

    // Count points inside slice.
    Eigen::MatrixXd::Index nPoints = 0;
    while (context_->query.next())
    {
        d = pointPlaneDistance(context_->query.x(),
                               context_->query.y(),
                               context_->query.z(),
                               point.x,
                               point.y,
                               point.z,
                               point.vx,
                               point.vy,
                               point.vz);
        if (d < m)
        {
            nPoints++;
        }
    }

    if (nPoints < 1)
    {
        return;
    }

    // Copy points inside slice and compute start estimate [x,y,z].
    Eigen::MatrixXd xyz;
    xyz.resize(3, nPoints);
    nPoints = 0;

    double x = 0;
    double y = 0;
    double z = 0;

    context_->query.reset();
    while (context_->query.next())
    {
        d = pointPlaneDistance(context_->query.x(),
                               context_->query.y(),
                               context_->query.z(),
                               point.x,
                               point.y,
                               point.z,
                               point.vx,
                               point.vy,
                               point.vz);

        if (d < m)
        {
            xyz(0, nPoints) = context_->query.x();
            xyz(1, nPoints) = context_->query.y();
            xyz(2, nPoints) = context_->query.z();

            x += context_->query.x();
            y += context_->query.y();
            z += context_->query.z();

            nPoints++;
        }
    }

    const double n = static_cast<double>(nPoints);
    x = x / n;
    y = y / n;
    z = z / n;

    // Compute L1 median.
    context_->median.median(xyz, x, y, z);

    point.x = x;
    point.y = y;
    point.z = z;
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
