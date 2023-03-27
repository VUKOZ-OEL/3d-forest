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

/** @file SegmentationL1TaskVoxelize.cpp */

#include <Editor.hpp>
#include <SegmentationL1TaskVoxelize.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskVoxelize"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskVoxelize::initialize(SegmentationL1Context *context)
{
    context_ = context;

    LOG_DEBUG(<< "Reset context.");
    context_->reset();

    context_->query.setWhere(context_->editor->viewports().where());
    context_->query.setVoxels(context_->parameters.voxelSize,
                              context_->editor->clipBoundary());

    context_->voxelFile.create("voxels.bin");

    uint64_t n = context_->query.numberOfVoxels();
    LOG_DEBUG(<< "n <" << n << ">.");

    ProgressActionInterface::initialize(n);
}

void SegmentationL1TaskVoxelize::next()
{
    uint64_t i = 0;

    startTimer();
    while (context_->query.nextVoxel())
    {
        step();

        i++;
        if (timedOut())
        {
            increment(i);
            return;
        }
    }

    context_->voxelFile.close();

    setProcessed(maximum());
}

void SegmentationL1TaskVoxelize::step()
{
    Query &query = context_->query;

    query.where().setBox(query.voxelBox());
    query.exec();

    if (!query.next())
    {
        return;
    }

    uint64_t nPoints = 1;
    VoxelFile::Voxel voxel;

    voxel.x = query.x();
    voxel.y = query.y();
    voxel.z = query.z();
    voxel.descriptor = query.descriptor();

    while (query.next())
    {
        voxel.x += query.x();
        voxel.y += query.y();
        voxel.z += query.z();

        nPoints++;
    }

    const double d = static_cast<double>(nPoints);
    voxel.x /= d;
    voxel.y /= d;
    voxel.z /= d;

    context_->voxelFile.write(voxel);
}
