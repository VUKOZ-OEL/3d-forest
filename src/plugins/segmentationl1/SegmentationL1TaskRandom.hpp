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

/** @file SegmentationL1TaskRandom.hpp */

#ifndef SEGMENTATION_L1_TASK_RANDOM_HPP
#define SEGMENTATION_L1_TASK_RANDOM_HPP

#include <Editor.hpp>
#include <SegmentationL1TaskInterface.hpp>

/** Segmentation L1 Task Random. */
class SegmentationL1TaskRandom : public SegmentationL1TaskInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;
        context_->query.reset();

        uint64_t max = context_->numberOfPoints;
        size_t n = 0;
        int c = context_->parameters.numberOfSamples;
        if (c > 0)
        {
            double p = static_cast<double>(c) * 0.01;
            n = static_cast<uint64_t>(static_cast<double>(max) * p);
        }
        if (n == 0 && max > 0)
        {
            n = 1;
        }
        context_->numberOfSamples = n;
        context_->samples.resize(n);

        sampleIndex_ = 0;
        pointsFrom_ = 0;
        pointsStep_ = 0;
        if (n > 0)
        {
            pointsStep_ = context_->numberOfPoints / n;
        }

        ProgressActionInterface::initialize(context_->numberOfSamples, 1000UL);
    }

    virtual void next()
    {
        uint64_t n = process();
        uint64_t i = 0;

        startTimer();

        while (i < n)
        {
            uint64_t r = static_cast<uint64_t>(rand());
            r = pointsFrom_ + (r % pointsStep_);
            context_->samples[sampleIndex_].index = r;

            sampleIndex_++;
            pointsFrom_ += pointsStep_;

            i++;
            if (timedOut())
            {
                break;
            }
        }

        increment(i);
    }

private:
    SegmentationL1Context *context_;
    size_t sampleIndex_;
    uint64_t pointsFrom_;
    uint64_t pointsStep_;
};

#endif /* SEGMENTATION_L1_TASK_RANDOM_HPP */
