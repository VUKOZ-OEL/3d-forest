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

/** @file Density.cpp */

#include <Density.hpp>
#include <Editor.hpp>
#include <Math.hpp>

#define MODULE_NAME "Density"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

Density::Density(Editor *editor)
    : editor_(editor),
      queryPoints_(editor),
      queryPoint_(editor),
      status_(STATUS_NEW)
{
    LOG_DEBUG_LOCAL();
}

Density::~Density()
{
    LOG_DEBUG_LOCAL();
}

int Density::start(double radius)
{
    LOG_DEBUG_LOCAL(<< "radius <" << radius << ">");

    radius_ = radius;

    status_ = STATUS_COMPUTE_DENSITY;

    densityMinimum_ = 0;
    densityMaximum_ = 0;

    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsPerStep_ = 5000;
    nPointsProcessed_ = 0;

    uint64_t nSteps = nPointsTotal_ / nPointsPerStep_;
    if (nPointsTotal_ % nPointsPerStep_ > 0)
    {
        nSteps++;
    }

    currentStep_ = 0;
    numberOfSteps_ = static_cast<int>(nSteps * 2);

    LOG_DEBUG_LOCAL(<< "numberOfSteps <" << numberOfSteps_ << ">");

    return numberOfSteps_;
}

void Density::step()
{
    LOG_DEBUG_LOCAL(<< "step <" << (currentStep_ + 1) << "> from <"
                    << numberOfSteps_ << ">");

    if (status_ == STATUS_COMPUTE_DENSITY)
    {
        stepComputeDensity();
        currentStep_++;
    }
    else if (status_ == STATUS_NORMALIZE_DENSITY)
    {
        stepNormalizeDensity();
        currentStep_++;
    }
}

void Density::stepComputeDensity()
{
    uint64_t nPointsToProcess = nPointsTotal_ - nPointsProcessed_;
    if (nPointsToProcess > nPointsPerStep_)
    {
        nPointsToProcess = nPointsPerStep_;
    }
    LOG_DEBUG_LOCAL(<< "points to process <" << nPointsToProcess << "> start <"
                    << nPointsProcessed_ << "> total <" << nPointsTotal_
                    << ">");

    if (nPointsProcessed_ == 0)
    {
        queryPoints_.where().setBox(editor_->clipBoundary());
        queryPoints_.exec();
    }

    uint64_t i = 0;

    while (i < nPointsToProcess && queryPoints_.next())
    {
        queryPoint_.where().setSphere(queryPoints_.x(),
                                      queryPoints_.y(),
                                      queryPoints_.z(),
                                      radius_);
        queryPoint_.exec();

        size_t nPointsSphere = 0;

        while (queryPoint_.next())
        {
            nPointsSphere++;
        }

        queryPoints_.value() = nPointsSphere;
        queryPoints_.setModified();

        if (i == 0 && nPointsProcessed_ == 0)
        {
            densityMinimum_ = nPointsSphere;
            densityMaximum_ = nPointsSphere;
        }
        else
        {
            updateRange(nPointsSphere, densityMinimum_, densityMaximum_);
        }

        i++;
    }

    nPointsProcessed_ += nPointsToProcess;
    LOG_DEBUG_LOCAL(<< "points processed <" << nPointsProcessed_ << "> from <"
                    << nPointsTotal_ << ">");

    if (nPointsProcessed_ == nPointsTotal_)
    {
        status_ = STATUS_NORMALIZE_DENSITY;
        nPointsProcessed_ = 0;
    }
}

void Density::stepNormalizeDensity()
{
    uint64_t nPointsToProcess = nPointsTotal_ - nPointsProcessed_;
    if (nPointsToProcess > nPointsPerStep_)
    {
        nPointsToProcess = nPointsPerStep_;
    }
    LOG_DEBUG_LOCAL(<< "points to process <" << nPointsToProcess << "> start <"
                    << nPointsProcessed_ << "> total <" << nPointsTotal_
                    << ">");

    if (nPointsProcessed_ == 0)
    {
        queryPoints_.reset();
    }

    size_t densityRange_ = densityMaximum_ - densityMinimum_;

    uint64_t i = 0;

    if (densityRange_ > 0)
    {
        double d = 1.0 / static_cast<double>(densityRange_);
        double v;

        while (i < nPointsToProcess && queryPoints_.next())
        {
            v = static_cast<double>(queryPoints_.value() - densityMinimum_);
            queryPoints_.density() = static_cast<float>(v * d);
            queryPoints_.setModified();
            i++;
        }
    }
    else
    {
        while (i < nPointsToProcess && queryPoints_.next())
        {
            queryPoints_.density() = 0;
            queryPoints_.setModified();
            i++;
        }
    }

    nPointsProcessed_ += nPointsToProcess;
    LOG_DEBUG_LOCAL(<< "points processed <" << nPointsProcessed_ << "> from <"
                    << nPointsTotal_ << ">");

    if (nPointsProcessed_ == nPointsTotal_)
    {
        status_ = STATUS_FINISHED;
        nPointsProcessed_ = 0;
        queryPoints_.flush();
    }
}

void Density::clear()
{
    LOG_DEBUG_LOCAL();

    queryPoints_.clear();
    queryPoint_.clear();

    status_ = STATUS_NEW;

    radius_ = 0;

    currentStep_ = 0;
    numberOfSteps_ = 0;

    nPointsTotal_ = 0;
    nPointsPerStep_ = 0;
    nPointsProcessed_ = 0;

    densityMinimum_ = 0;
    densityMaximum_ = 0;
}
