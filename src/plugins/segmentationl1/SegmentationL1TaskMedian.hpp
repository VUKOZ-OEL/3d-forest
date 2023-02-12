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

/** @file SegmentationL1TaskMedian.hpp */

#ifndef SEGMENTATION_L1_TASK_MEDIAN_HPP
#define SEGMENTATION_L1_TASK_MEDIAN_HPP

#include <Editor.hpp>
#include <SegmentationL1TaskInterface.hpp>

/** Segmentation L1 Task Median. */
class SegmentationL1TaskMedian : public SegmentationL1TaskInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;
        index_ = 0;
        r1_ =
            static_cast<double>(context_->parameters.neighborhoodRadiusMinimum);
        r2_ =
            static_cast<double>(context_->parameters.neighborhoodRadiusMaximum);

        ProgressActionInterface::initialize(context_->samples.size());
    }

    virtual void next()
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

    void step()
    {
        SegmentationL1Point &point = context_->samples[index_];
        double x = point.x - (point.nx * r2_);
        double y = point.y - (point.ny * r2_);
        double z = point.z - (point.nz * r2_);

        context_->query.where().setSphere(x, y, z, r2_ + (r2_ * 0.1));
        context_->query.exec();
        context_->query.mean(x, y, z);

        point.x = x;
        point.y = y;
        point.z = z;

        index_++;
    }

private:
    SegmentationL1Context *context_;
    size_t index_;
    double r1_;
    double r2_;
};

#endif /* SEGMENTATION_L1_TASK_MEDIAN_HPP */
