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

/** @file ComputeCrownVolumeAction.cpp */

// Include std.
#include <algorithm>

// Include 3D Forest.
#include <ColorPalette.hpp>
#include <ComputeCrownVolumeAction.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeCrownVolumeAction"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_CROWN_VOLUME_STEP_POINTS_TO_VOXELS 0
#define COMPUTE_CROWN_VOLUME_STEP_CALCULATE_VOLUME 1

ComputeCrownVolumeAction::ComputeCrownVolumeAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeCrownVolumeAction::~ComputeCrownVolumeAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeCrownVolumeAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    grid_.clear();
    treeIdGridMinZ_.clear();
}

void ComputeCrownVolumeAction::start(
    const ComputeCrownVolumeParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    ppm_ = editor_->settings().unitsSettings().pointsPerMeter()[0];

    parameters_ = parameters;

    parameters_.voxelSize *= ppm_;

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;
    grid_.clear();
    treeIdGridMinZ_.clear();

    // Plan the steps.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps({25.0, 25.0, 25.0, 25.0});
    progress_.setValueSteps(COMPUTE_CROWN_VOLUME_STEP_POINTS_TO_VOXELS);
}

void ComputeCrownVolumeAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_CROWN_VOLUME_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case COMPUTE_CROWN_VOLUME_STEP_CALCULATE_VOLUME:
            stepCalculateVolume();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeCrownVolumeAction::stepPointsToVoxels()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to use active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    const Segments &segments = editor_->segments();

    // For each point in filtered datasets:
    while (query_.next())
    {
        size_t treeId = query_.segment();
        if (treeId > 0)
        {
            size_t segmentIndex = segments.index(treeId, false);
            if (segmentIndex != SIZE_MAX)
            {
                const Segment &segment = segments[segmentIndex];
                double h = query_.z() - segment.boundary.min(2);
                if (h >= segment.treeAttributes.crownStartHeight)
                {
                    // Create new voxel.
                    createVoxel();
                }
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(1, 1);
    progress_.setValueSteps(COMPUTE_CROWN_VOLUME_STEP_CALCULATE_VOLUME);
}

void ComputeCrownVolumeAction::stepCalculateVolume()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Start calculating volume for trees.");
    }

    // Get copy of current segments.
    Segments segments = editor_->segments();

    // Reset tree attribute values.
    for (size_t i = 0; i < segments.size(); i++)
    {
        segments[i].treeAttributes.crownVoxelCountPerMeters.clear();
        segments[i].treeAttributes.crownVoxelCount = 0;
        segments[i].treeAttributes.crownVoxelCountShared.clear();
        segments[i].treeAttributes.crownVoxelSize = 0;
    }

    LOG_DEBUG(<< "Grid size <" << grid_.size() << ">.");

    // Iterate the grid.
    for (const auto &[k, v] : grid_)
    {
        // Get grid voxel.
        const auto &[qx, qy, qz] = k;

        // The number of trees in the voxel.
        size_t nTrees = v.treeIdList.size();

        // LOG_DEBUG(<< "Voxel [" << qx << ", " << qy << ", " << qz << "]"
        //           << "size <" << v.treeIdList.size() << ">.");

        // Iterate all tree ids in the voxel.
        for (size_t treeId : v.treeIdList)
        {
            // Get tree and tree attributes from the cell.
            size_t segmentIndex = segments.index(treeId, false);
            if (segmentIndex == SIZE_MAX)
            {
                continue;
            }

            Segment &segment = segments[segmentIndex];
            TreeAttributes &atr = segment.treeAttributes;

            // Increment total voxel count.
            atr.crownVoxelCount++;

            // Process shared tree voxels.
            if (nTrees > 1)
            {
                for (size_t otherTreeId : v.treeIdList)
                {
                    if (otherTreeId != treeId)
                    {
                        atr.crownVoxelCountShared[otherTreeId]++;
                    }
                }
            }

            // Store the number of voxels per each meter.
            auto itMin = treeIdGridMinZ_.find(treeId);
            if (itMin != treeIdGridMinZ_.end())
            {
                size_t qh = qz - itMin->second;
                double hppm = static_cast<double>(qh) * parameters_.voxelSize;
                size_t h = static_cast<size_t>(hppm / ppm_); // To meters.
                size_t n = atr.crownVoxelCountPerMeters.size();
                if (h >= n)
                {
                    atr.crownVoxelCountPerMeters.resize(h + 1);
                    for (size_t i = n; i < h + 1; i++)
                    {
                        atr.crownVoxelCountPerMeters[i] = 0;
                    }
                }

                atr.crownVoxelCountPerMeters[h]++;
            }
        }
    }

    // Set attributes.
    for (size_t i = 0; i < segments.size(); i++)
    {
        TreeAttributes &atr = segments[i].treeAttributes;

        // Store voxel size which was used for the calculation.
        atr.crownVoxelSize = parameters_.voxelSize;
    }

    // Set new segments to editor.
    editor_->setSegments(segments);

    // Finish.
    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());

    LOG_DEBUG(<< "Finished calculating volume for trees.");
}

void ComputeCrownVolumeAction::createVoxel()
{
    // Initialize new voxel point.
    size_t treeId = query_.segment();

    const Box<double> &box = editor_->boundary();
    double xr = query_.x() - box.min(0);
    double yr = query_.y() - box.min(1);
    double zr = query_.z() - box.min(2);

    double step = parameters_.voxelSize;

    int qx = static_cast<int>(xr / step);
    int qy = static_cast<int>(yr / step);
    int qz = static_cast<int>(zr / step);

    grid_[{qx, qy, qz}].treeIdList.insert(treeId);

    auto itMin = treeIdGridMinZ_.find(treeId);
    if (itMin != treeIdGridMinZ_.end())
    {
        if (qz < itMin->second)
        {
            itMin->second = qz;
        }
    }
    else
    {
        treeIdGridMinZ_[treeId] = qz;
    }
}
