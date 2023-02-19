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

/** @file SegmentationL1TaskNormal.hpp */

#ifndef SEGMENTATION_L1_TASK_NORMAL_HPP
#define SEGMENTATION_L1_TASK_NORMAL_HPP

#include <Editor.hpp>
#include <SegmentationL1TaskInterface.hpp>

/** Segmentation L1 Task Normal. */
class SegmentationL1TaskNormal : public SegmentationL1TaskInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;
        context_->query.setWhere(context_->editor->viewports().where());
        context_->query.exec();

        index_ = 0;
        radius_ =
            static_cast<double>(context_->parameters.neighborhoodRadiusMinimum);

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
        point.hasVectors = context_->pca.normal(context_->query,
                                                point.x,
                                                point.y,
                                                point.z,
                                                radius_,
                                                point.nx,
                                                point.ny,
                                                point.nz,
                                                point.vx,
                                                point.vy,
                                                point.vz);

        if (!point.hasVectors)
        {
            point.nx = 0;
            point.ny = 0;
            point.nz = 0;
            point.vx = 0;
            point.vy = 0;
            point.vz = 0;
        }

        index_++;
    }

private:
    SegmentationL1Context *context_;
    size_t index_;
    double radius_;
};

#endif /* SEGMENTATION_L1_TASK_NORMAL_HPP */
