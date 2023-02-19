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

/** @file SegmentationL1TaskNormal.cpp */

#include <Editor.hpp>
#include <SegmentationL1TaskNormal.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskNormal"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskNormal::initialize(SegmentationL1Context *context)
{
    context_ = context;

    context_->samples = context_->samplesBackup;
    context_->query.setWhere(context_->editor->viewports().where());
    context_->query.exec();

    index_ = 0;
    radius_ =
        static_cast<double>(context_->parameters.neighborhoodRadiusMinimum);

    ProgressActionInterface::initialize(context_->samples.size());
}

void SegmentationL1TaskNormal::next()
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

void SegmentationL1TaskNormal::step()
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

    if (index_ == context_->samples.size())
    {
        context_->samplesBackup = context_->samples;
    }
}
