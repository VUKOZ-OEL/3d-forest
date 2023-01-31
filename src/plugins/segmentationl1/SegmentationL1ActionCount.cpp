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

/** @file SegmentationL1ActionCount.cpp */

#include <Editor.hpp>
#include <SegmentationL1ActionCount.hpp>

#define LOG_MODULE_NAME "SegmentationL1ActionCount"
#include <Log.hpp>

SegmentationL1ActionCount::SegmentationL1ActionCount() : context_(nullptr)
{
    LOG_DEBUG(<< "Create.");
}

SegmentationL1ActionCount::~SegmentationL1ActionCount()
{
    LOG_DEBUG(<< "Destroy.");
}

void SegmentationL1ActionCount::initialize(SegmentationL1Context *context)
{
    LOG_DEBUG(<< "Initialize.");

    context_ = context;

    context_->reset();

    context_->query.setWhere(context_->editor->viewports().where());
    context_->query.exec();

    if (context_->query.next())
    {
        context_->totalSamplesCount++;
    }

    ProgressActionInterface::initialize(ProgressActionInterface::npos, 1000UL);
}

void SegmentationL1ActionCount::step()
{
    if (initializing())
    {
        determineMaximum();
        return;
    }

    uint64_t i = 0;

    startTimer();

    while (context_->query.next())
    {
        i++;

        if (timedOut())
        {
            break;
        }
    }

    increment(i);
}

void SegmentationL1ActionCount::determineMaximum()
{
    // Compute total number of points.
    startTimer();

    while (context_->query.next())
    {
        context_->totalSamplesCount++;

        if (timedOut())
        {
            return;
        }
    }

    context_->query.reset();

    // Compute number of initial sample points.
    uint64_t max = context_->totalSamplesCount;
    size_t n = 0;
    int c = context_->parameters.initialSamplesCount;
    if (c > 0)
    {
        double p = static_cast<double>(c) * 0.01;
        n = static_cast<uint64_t>(static_cast<double>(max) * p);
    }
    context_->initialSamplesCount = n;
    context_->points.resize(n);

    for (size_t i = 0; i < n; i++)
    {
        uint64_t r = static_cast<uint64_t>(rand()) % max;
        context_->points[i].index = r;
    }

    LOG_DEBUG(<< "The number of initial sample points is <"
              << context_->initialSamplesCount << ">.");

    ProgressActionInterface::initialize(context_->totalSamplesCount, 1000UL);
}
