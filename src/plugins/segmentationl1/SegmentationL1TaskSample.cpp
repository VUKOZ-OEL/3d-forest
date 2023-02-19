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

    voxelsStep_ = 0;
    sampleIndex_ = 0;

    context_->voxelFile.open("voxels.bin");
    setNumberOfSamples();

    ProgressActionInterface::initialize(context_->samples.size(), 1000U);
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
            break;
        }
    }

    increment(i);
}

void SegmentationL1TaskSample::step()
{
    if (sampleIndex_ >= context_->samples.size())
    {
        return;
    }

    uint64_t r = static_cast<uint64_t>(rand()) % voxelsStep_;

    VoxelFile::Voxel voxel;
    context_->voxelFile.skip(r);
    context_->voxelFile.read(voxel);
    context_->voxelFile.skip(voxelsStep_ - (r + 1U));

    context_->samples[sampleIndex_].x = voxel.x;
    context_->samples[sampleIndex_].y = voxel.y;
    context_->samples[sampleIndex_].z = voxel.z;

    LOG_DEBUG(<< "Add sample <" << (sampleIndex_ + 1U) << "/"
              << context_->samples.size() << "> data <" << voxel << ">.");

    sampleIndex_++;
}

void SegmentationL1TaskSample::setNumberOfSamples()
{
    uint64_t nVoxels = context_->voxelFile.nVoxels();

    size_t n = 0;
    int c = context_->parameters.numberOfSamples;
    if (c > 0)
    {
        double p = static_cast<double>(c) * 0.01;
        n = static_cast<size_t>(static_cast<double>(nVoxels) * p);
    }

    if (n == 0 && nVoxels > 0)
    {
        n = 1;
    }

    context_->samples.resize(n);

    if (n > 0)
    {
        voxelsStep_ = nVoxels / n;
    }

    LOG_DEBUG(<< "numberOfVoxels <" << nVoxels << "> numberOfSamples <"
              << context_->samples.size() << "> step <" << voxelsStep_ << ">.");
}
