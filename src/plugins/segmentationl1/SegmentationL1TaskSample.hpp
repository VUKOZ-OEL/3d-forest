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

/** @file SegmentationL1TaskSample.hpp */

#ifndef SEGMENTATION_L1_TASK_SAMPLE_HPP
#define SEGMENTATION_L1_TASK_SAMPLE_HPP

#include <Editor.hpp>
#include <SegmentationL1TaskInterface.hpp>

/** Segmentation L1 Task Sample. */
class SegmentationL1TaskSample : public SegmentationL1TaskInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;
        context_->query.reset();
        sampleIndex_ = 0;
        pointsIndex_ = 0;

        ProgressActionInterface::initialize(context_->numberOfPoints, 1000UL);
    }

    virtual void next()
    {
        uint64_t n = process();
        uint64_t i = 0;

        startTimer();

        while (i < n)
        {
            if (sampleIndex_ < context_->samples.size() &&
                context_->query.next() &&
                context_->samples[sampleIndex_].index == pointsIndex_)
            {
                context_->samples[sampleIndex_].x = context_->query.x();
                context_->samples[sampleIndex_].y = context_->query.y();
                context_->samples[sampleIndex_].z = context_->query.z();
                sampleIndex_++;
            }

            pointsIndex_++;

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
    uint64_t pointsIndex_;
};

#endif /* SEGMENTATION_L1_TASK_SAMPLE_HPP */