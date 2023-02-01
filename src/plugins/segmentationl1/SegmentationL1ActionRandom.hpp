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

/** @file SegmentationL1ActionRandom.hpp */

#ifndef SEGMENTATION_L1_ACTION_RANDOM_HPP
#define SEGMENTATION_L1_ACTION_RANDOM_HPP

#include <Editor.hpp>
#include <SegmentationL1ActionInterface.hpp>

/** Segmentation L1 Action Random. */
class SegmentationL1ActionRandom : public SegmentationL1ActionInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;

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
        context_->query.reset();

        initialSampleIndex_ = 0;

        ProgressActionInterface::initialize(context_->initialSamplesCount,
                                            1000UL);
    }

    virtual void step()
    {
        uint64_t n = process();
        uint64_t i = 0;

        startTimer();

        while (i < n)
        {
            uint64_t r =
                static_cast<uint64_t>(rand()) % context_->totalSamplesCount;
            context_->points[initialSampleIndex_].index = r;

            i++;
            initialSampleIndex_++;

            if (timedOut())
            {
                break;
            }
        }

        increment(i);
    }

private:
    SegmentationL1Context *context_;
    size_t initialSampleIndex_;
};

#endif /* SEGMENTATION_L1_ACTION_RANDOM_HPP */
