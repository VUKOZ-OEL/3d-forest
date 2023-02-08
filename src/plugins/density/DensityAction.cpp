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

/** @file DensityAction.cpp */

#include <DensityAction.hpp>
#include <Editor.hpp>
#include <Math.hpp>

#define LOG_MODULE_NAME "DensityAction"
#include <Log.hpp>

DensityAction::DensityAction(Editor *editor)
    : editor_(editor),
      queryPoints_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Called.");
}

DensityAction::~DensityAction()
{
    LOG_DEBUG(<< "Called.");
}

void DensityAction::clear()
{
    LOG_DEBUG(<< "Called.");

    queryPoints_.clear();
    queryPoint_.clear();

    status_ = STATUS_NEW;

    radius_ = 0;

    nPointsTotal_ = 0;

    densityMinimum_ = 0;
    densityMaximum_ = 0;
}

void DensityAction::initialize(double radius)
{
    LOG_DEBUG(<< "Called with parameter radius <" << radius << ">.");

    radius_ = radius;

    status_ = STATUS_COMPUTE_DENSITY;

    nPointsTotal_ = 0;
    nPointsOneHalf_ = 0;
    nPointsDone_ = 0;

    densityMinimum_ = 0;
    densityMaximum_ = 0;

    queryPoints_.setWhere(editor_->viewports().where());

    queryPoints_.exec();

    if (queryPoints_.next())
    {
        nPointsTotal_++;
    }

    ProgressActionInterface::initialize(ProgressActionInterface::npos, 1000UL);
}

void DensityAction::next()
{
    if (initializing())
    {
        determineMaximum();
    }
    else if (status_ == STATUS_COMPUTE_DENSITY)
    {
        stepComputeDensity();
    }
    else if (status_ == STATUS_NORMALIZE_DENSITY)
    {
        stepNormalizeDensity();
    }
}

void DensityAction::determineMaximum()
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

void DensityAction::stepComputeDensity()
{
    uint64_t i = 0;

    startTimer();

    while (nPointsDone_ < nPointsOneHalf_)
    {
        if (queryPoints_.next())
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

            if (i == 0 && processed() == 0)
            {
                densityMinimum_ = nPointsSphere;
                densityMaximum_ = nPointsSphere;
            }
            else
            {
                updateRange(nPointsSphere, densityMinimum_, densityMaximum_);
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
        LOG_DEBUG(<< "Change state to STATUS_NORMALIZE_DENSITY.");
        status_ = STATUS_NORMALIZE_DENSITY;
        queryPoints_.reset();
    }
}

void DensityAction::stepNormalizeDensity()
{
    uint64_t i = 0;

    startTimer();

    size_t densityRange_ = densityMaximum_ - densityMinimum_;

    if (densityRange_ > 0)
    {
        double d = 1.0 / static_cast<double>(densityRange_);
        double v;

        while (nPointsDone_ < nPointsTotal_)
        {
            if (queryPoints_.next())
            {
                v = static_cast<double>(queryPoints_.value() - densityMinimum_);
                queryPoints_.density() = static_cast<float>(v * d);
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
                queryPoints_.density() = 0;
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
