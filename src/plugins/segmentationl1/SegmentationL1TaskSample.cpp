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

/** @file SegmentationL1TaskSample.cpp */

#include <Editor.hpp>
#include <SegmentationL1TaskSample.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskSample"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskSample::initialize(SegmentationL1Context *context)
{
    context_ = context;

    setNumberOfSamples();

    context_->execInitialSamplesQuery();

    ProgressActionInterface::initialize(context_->samples.size());
}

void SegmentationL1TaskSample::next()
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
            increment(i);
            return;
        }
    }

    context_->samplesBackup = context_->samples;

    setProcessed(maximum());
}

void SegmentationL1TaskSample::step()
{
    uint64_t j = 0;
    uint64_t r = static_cast<uint64_t>(rand()) % interval_;

    while (j < r && context_->query.next())
    {
        j++;
    }

    if (context_->query.next())
    {
        context_->samples[index_].x = context_->query.x();
        context_->samples[index_].y = context_->query.y();
        context_->samples[index_].z = context_->query.z();
        j++;
    }

    index_++;
    if (index_ == context_->samples.size())
    {
        return;
    }

    while (j < interval_ && context_->query.next())
    {
        j++;
    }
}

void SegmentationL1TaskSample::setNumberOfSamples()
{
    size_t n = static_cast<size_t>(context_->parameters.numberOfSamples);
#if 0
    size_t n = static_cast<size_t>(
        static_cast<double>(context_->nPoints) *
        static_cast<double>(context_->parameters.numberOfSamples) * 0.01);

    LOG_DEBUG(<< "Set number of samples to <" << n << "> as <"
              << context_->parameters.numberOfSamples << "> percent from <"
              << context_->nPoints << "> points.");
#endif
    if (n == 0 && context_->nPoints > 0)
    {
        n = 1;
    }

    context_->samples.resize(n);
    std::memset(context_->samples.data(), 0, sizeof(SegmentationL1Point) * n);
    context_->samplesBackup = context_->samples;

    index_ = 0;

    if (n > 0)
    {
        interval_ = context_->nPoints / n;
    }
    else
    {
        interval_ = 0;
    }
}
