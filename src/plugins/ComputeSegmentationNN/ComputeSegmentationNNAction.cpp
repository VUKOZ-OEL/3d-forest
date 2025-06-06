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

/** @file ComputeSegmentationNNAction.cpp */

// Include 3D Forest.
#include <ComputeSegmentationNNAction.hpp>
#include <Editor.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeSegmentationNNAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define COMPUTE_SEGMENTATION_NN_STEP_RESET_POINTS 0
#define COMPUTE_SEGMENTATION_NN_STEP_COUNT_POINTS 1
#define COMPUTE_SEGMENTATION_NN_STEP_POINTS_TO_VOXELS 2
#define COMPUTE_SEGMENTATION_NN_STEP_CREATE_VOXEL_INDEX 3
#define COMPUTE_SEGMENTATION_NN_STEP_CREATE_TRUNKS 4
#define COMPUTE_SEGMENTATION_NN_STEP_CREATE_BRANCHES 5
#define COMPUTE_SEGMENTATION_NN_STEP_CREATE_SEGMENTS 6
#define COMPUTE_SEGMENTATION_NN_STEP_VOXELS_TO_POINTS 7

ComputeSegmentationNNAction::ComputeSegmentationNNAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

ComputeSegmentationNNAction::~ComputeSegmentationNNAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void ComputeSegmentationNNAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    nPointsTotal_ = 0;
    nPointsInFilter_ = 0;

    voxels_.clear();
    groups_.clear();
    path_.clear();
}

void ComputeSegmentationNNAction::Group::clear()
{
    segmentId = 0;
    nPoints = 0;
    boundary.clear();
    averagePoint.clear();
}

void ComputeSegmentationNNAction::start(
    const ComputeSegmentationNNParameters &parameters)
{
    LOG_DEBUG(<< "Start with parameters <" << toString(parameters) << ">.");

    // Set input parameters.
    double ppm = editor_->settings().unitsSettings().pointsPerMeter()[0];
    LOG_DEBUG(<< "Units pointsPerMeter <" << ppm << ">.");

    parameters_ = parameters;

    parameters_.voxelRadius *= ppm;
    parameters_.woodThresholdMin *= 0.01; // %
    parameters_.searchRadiusTrunkPoints *= ppm;
    parameters_.searchRadiusLeafPoints *= ppm;
    parameters_.treeBaseElevationMin *= ppm;
    parameters_.treeBaseElevationMax *= ppm;
    parameters_.treeHeightMin *= ppm;

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;

    voxels_.clear();
    groups_.clear();
    path_.clear();

    // Plan the steps.
    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps({4.0, 1.0, 24.0, 1.0, 25.0, 35.0, 1.0, 9.0});
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_RESET_POINTS);
}

void ComputeSegmentationNNAction::next()
{
    switch (progress_.valueSteps())
    {
        case COMPUTE_SEGMENTATION_NN_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_CREATE_VOXEL_INDEX:
            stepCreateVoxelIndex();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_CREATE_TRUNKS:
            stepCreateTrunks();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_CREATE_BRANCHES:
            stepCreateBranches();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_CREATE_SEGMENTS:
            stepCreateSegments();
            break;

        case COMPUTE_SEGMENTATION_NN_STEP_VOXELS_TO_POINTS:
            stepVoxelsToPoints();
            break;

        default:
            // Empty.
            break;
    }
}

void ComputeSegmentationNNAction::stepResetPoints()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        LOG_DEBUG(<< "Reset all <" << nPointsTotal_ << "> points.");

        // Initialize. Remove all segments and create default main segment.
        Segments segments;
        segments.setDefault();

        QueryFilterSet segmentsFilter;
        segmentsFilter.clear();
        segmentsFilter.setEnabled(0, true);
        segmentsFilter.setEnabled(true);

        editor_->setSegments(segments);
        editor_->setSegmentsFilter(segmentsFilter);

        // Set query to iterate all points. Active filter is ignored.
        query_.setWhere(QueryWhere());
        query_.exec();
    }

    // For each point in all datasets:
    while (query_.next())
    {
        // Set point index to voxel to none.
        query_.voxel() = SIZE_MAX;

        // Set point segment to 'unsegmented' segment.
        query_.segment() = 0;

        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_COUNT_POINTS);
}

void ComputeSegmentationNNAction::stepCountPoints()
{
    progress_.startTimer();

    // Initialize.
    if (progress_.valueStep() == 0)
    {
        // Set query to use active filter.
        query_.setWhere(editor_->viewports().where());
        query_.exec();
    }

    // Count the number of filtered points.
    while (query_.next())
    {
        nPointsInFilter_++;

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Counted <" << nPointsInFilter_ << "> points in filter.");

    query_.reset();

    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_POINTS_TO_VOXELS);
}

void ComputeSegmentationNNAction::stepPointsToVoxels()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point index to voxel is none:
        if (query_.voxel() == SIZE_MAX &&
            query_.classification() != LasFile::CLASS_GROUND &&
            (parameters_.zCoordinatesAsElevation ||
             (query_.elevation() >= parameters_.treeBaseElevationMin)))
        {
            // Create new voxel.
            createVoxel();
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Created <" << voxels_.size() << "> voxels.");
    // voxels_.exportToFile("voxels.json");

    query_.reset();

    progress_.setMaximumStep(voxels_.size(), 100);
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_CREATE_VOXEL_INDEX);
}

void ComputeSegmentationNNAction::stepCreateVoxelIndex()
{
    // Create voxel index.
    // TBD: Use some iterative algorithm with increasing progress.
    voxels_.createIndex();

    LOG_DEBUG(<< "Created voxel index.");

    progress_.setMaximumStep(voxels_.size(), 10);
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_CREATE_TRUNKS);
}

void ComputeSegmentationNNAction::stepCreateTrunks()
{
    progress_.startTimer();

    // If it is the first call, initialize:
    if (progress_.valueStep() == 0)
    {
        // Start from the first voxel.
        pointIndex_ = 0;
        // Set group id to zero.
        groupId_ = 0;
        group_.clear();
        // Set the path and the group empty.
        path_.resize(0);
        groupPath_.resize(0);
    }

    // Repeat until all voxels and the last path are processed:
    while (pointIndex_ < voxels_.size() || path_.size() > 0)
    {
        // If the path is empty, try to start new path:
        if (path_.empty())
        {
            LOG_DEBUG(<< "Start next path.");
            // If a voxel is not processed and meets
            // criteria (for wood), add it to the path.
            if (trunkVoxel(voxels_[pointIndex_]))
            {
                LOG_DEBUG(<< "Start next trunk group.");
                startGroup(voxels_[pointIndex_], true);
                voxels_[pointIndex_].group = groupId_;
                path_.push_back(pointIndex_);
            }

            // Move to the next voxel.
            pointIndex_++;
            progress_.addValueStep(1);
        }
        // Else the path is being processed:
        else
        {
            // Add the path to the current group.
            LOG_DEBUG(<< "Add path with <" << path_.size() << "> points.");
            size_t idx = groupPath_.size();
            for (size_t i = 0; i < path_.size(); i++)
            {
                groupPath_.push_back(path_[i]);
            }

            // Set the path empty.
            path_.resize(0);

            // Try to expand the current group with neighbor voxels:
            for (size_t i = idx; i < groupPath_.size(); i++)
            {
                Point &a = voxels_[groupPath_[i]];
                voxels_.findRadius(a.x,
                                   a.y,
                                   a.z,
                                   parameters_.searchRadiusTrunkPoints,
                                   search_);
                for (size_t j = 0; j < search_.size(); j++)
                {
                    Point &b = voxels_[search_[j]];
                    // If a voxel in search radius is not processed and meets
                    // criteria (for wood), add it to group expansion.
                    if (trunkVoxel(b))
                    {
                        continueGroup(b, true);
                        b.group = groupId_;
                        path_.push_back(search_[j]);
                    }
                }
            }

            // If there are no other voxels for group expansion:
            if (path_.empty())
            {
                // If the current group meets some criteria:
                double groupHeight = groupMaximum_ - groupMinimum_;
                if (!(groupHeight < parameters_.treeHeightMin) &&
                    groupMinimum_ < parameters_.treeBaseElevationMax)
                {
                    // Mark this group as future segment.
                    groups_[groupId_] = group_;

                    // Increment group id by one.
                    groupId_++;
                }
                // Else throw away the current group:
                else
                {
                    // Set all voxels from the group as not processed.
                    for (size_t i = 0; i < groupPath_.size(); i++)
                    {
                        voxels_[groupPath_[i]].group = SIZE_MAX;
                    }
                }

                // Prepare start of the next group. Set the group empty.
                group_.clear();
                groupPath_.resize(0);
            }
        }

        if (progress_.timedOut())
        {
            return;
        }
    }

    if (parameters_.segmentOnlyTrunks)
    {
        progress_.setMaximumStep(nPointsInFilter_, 1000);
        progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_VOXELS_TO_POINTS);
    }
    else
    {
        progress_.setMaximumStep(voxels_.size(), 10);
        progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_CREATE_BRANCHES);
    }
}

void ComputeSegmentationNNAction::stepCreateBranches()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        // Start from the first voxel.
        pointIndex_ = 0;
        // Reset group.
        // Group id is the next unused group id value.
        group_.clear();
        groupUnsegmented_.clear();
        // Set the path empty.
        path_.resize(0);
    }

    // Repeat until all voxels are processed and the path is not finished:
    while (pointIndex_ < voxels_.size() || path_.size() > 0)
    {
        // If the current path is finished:
        if (path_.empty())
        {
            // If the current voxel V is not processed, start new path from V:
            if (voxels_[pointIndex_].group == SIZE_MAX)
            {
                // Find nearest unprocessed point U from V. Set V.next to U.
                // Set group of V to group id.
                startGroup(voxels_[pointIndex_]);
                voxels_[pointIndex_].group = groupId_;
                findNearestNeighbor(voxels_[pointIndex_]);

                // Append V into the current path.
                path_.push_back(pointIndex_);
            }

            // Move to the next voxel.
            pointIndex_++;
            progress_.addValueStep(1);
        }
        // Else the current path is being processed:
        else
        {
            // Find voxel U, where U is minimal distance V.next in the path.
            double dist = std::numeric_limits<double>::max();
            size_t nextIdx = SIZE_MAX;
            for (size_t i = 0; i < path_.size(); i++)
            {
                const Points &points = voxels_;
                const Point &a = points[path_[i]];
                if (a.next != SIZE_MAX && a.dist < dist)
                {
                    dist = a.dist;
                    nextIdx = a.next;
                }
            }

            // If the next nearest neighbor U is not found, terminate the path:
            if (nextIdx == SIZE_MAX)
            {
                // It was not possible to connect this path.
                // Merge the path to unsegmented group.
                // Set the path as finished.
                mergeToGroup(groupUnsegmented_, group_);
                path_.resize(0);
                group_.clear();

                // Increment group id for the next path by one.
                groupId_++;
            }
            // Else nearest neighbor U is found, expand the path:
            else
            {
                Point &a = voxels_[nextIdx];
                // If nearest neighbor U belongs to a group, connect the whole
                // path to this group:
                if (a.group != SIZE_MAX)
                {
                    // Set all voxels in the path to the same group as U.
                    for (size_t i = 0; i < path_.size(); i++)
                    {
                        Point &b = voxels_[path_[i]];
                        b.group = a.group;
                    }

                    // Merge current group to group of U.
                    mergeToGroup(groups_[a.group], group_);

                    // Set the path as finished.
                    path_.resize(0);
                    group_.clear();

                    // Increment group id for the next path by one.
                    groupId_++;
                }
                // Else nearest neighbor U does not belong to any group,
                // expand the path with new voxel U:
                else
                {
                    // Append U into the path.
                    continueGroup(a);
                    a.group = voxels_[path_[0]].group;
                    path_.push_back(nextIdx);

                    // Find nearest unprocessed point W from U. Set U.next to W.
                    findNearestNeighbor(a);

                    // Update nearest neighbors in the path.
                    // Find new nearest unprocessed neighbor V.next for all
                    // voxels which have V.next equal to U.
                    for (size_t i = 0; i < path_.size(); i++)
                    {
                        Point &b = voxels_[path_[i]];
                        if (b.next != SIZE_MAX && b.next == nextIdx)
                        {
                            findNearestNeighbor(b);
                        }
                    }
                }
            }
        }

        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_VOXELS_TO_POINTS);
}

void ComputeSegmentationNNAction::stepVoxelsToPoints()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        // Set segment id to all final groups.
        size_t segmentId = 1;
        for (auto &it : groups_)
        {
            it.second.boundary.clear();
            it.second.segmentId = segmentId;
            segmentId++;
        }
    }

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point belongs to some voxel:
        size_t pointIndex = query_.voxel();
        if (pointIndex < voxels_.size())
        {
            // If voxel's group belongs to a segment:
            size_t groupIndex = voxels_[pointIndex].group;
            auto it = groups_.find(groupIndex);
            if (it != groups_.end())
            {
                // Set point segment to the same value as voxel segment.
                query_.segment() = it->second.segmentId;
                query_.setModified();

                // Extend group boundary.
                it->second.boundary.extend(query_.x(), query_.y(), query_.z());
            }
        }

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    LOG_DEBUG(<< "Done.");

    query_.flush();

    progress_.setMaximumStep();
    progress_.setValueSteps(COMPUTE_SEGMENTATION_NN_STEP_CREATE_SEGMENTS);
}

void ComputeSegmentationNNAction::stepCreateSegments()
{
    LOG_DEBUG(<< "Create <" << groups_.size() << "> segments.");

    // Initialize new segments.
    Segments segments;

    segments.setDefault();
    segments[0].boundary = groupUnsegmented_.boundary;
    // segments[0].position = groupUnsegmented_.boundary.center();
    // segments[0].height = groupUnsegmented_.boundary.length(2);

    QueryFilterSet segmentsFilter;
    segmentsFilter.setEnabled(0, true);
    segmentsFilter.setEnabled(true);

    // For each final group, perform the following:
    for (auto &it : groups_)
    {
        size_t segmentId = it.second.segmentId;

        // Create new segment.
        segments.addTree(segmentId, it.second.boundary);

        // Enable new segment in segment filter.
        segmentsFilter.setEnabled(segmentId, true);
    }

    // Set new segments to editor.
    editor_->setSegments(segments);
    editor_->setSegmentsFilter(segmentsFilter);

    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void ComputeSegmentationNNAction::createVoxel()
{
    // Mark index of new voxel in voxel array.
    size_t idx = voxels_.size();

    // Initialize new voxel point.
    Point p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    p.elevation = 0;
    p.descriptor = 0;
    p.dist = 0;
    p.next = SIZE_MAX;
    p.group = SIZE_MAX;
    p.status = 0;

    // Compute point coordinates as average from all neighbour points.
    // Set value of each neighbour point to index of new voxel.
    size_t n = 0;

    queryPoint_.where().setSphere(query_.x(),
                                  query_.y(),
                                  query_.z(),
                                  parameters_.voxelRadius);
    queryPoint_.exec();

    while (queryPoint_.next())
    {
        if (queryPoint_.classification() == LasFile::CLASS_GROUND)
        {
            continue;
        }

        p.x += queryPoint_.x();
        p.y += queryPoint_.y();
        p.z += queryPoint_.z();
        p.elevation += queryPoint_.elevation();

        if (parameters_.leafToWoodChannel ==
            ComputeSegmentationNNParameters::CHANNEL_DESCRIPTOR)
        {
            if (queryPoint_.descriptor() > p.descriptor)
            {
                p.descriptor = queryPoint_.descriptor();
            }
        }
        else if (parameters_.leafToWoodChannel ==
                 ComputeSegmentationNNParameters::CHANNEL_INTENSITY)
        {
            if (queryPoint_.intensity() > p.descriptor)
            {
                p.descriptor = queryPoint_.intensity();
            }
        }
        else
        {
            THROW("ComputeSegmentationNNParameters leafToWoodChannel not "
                  "implemented.");
        }

        n++;

        queryPoint_.voxel() = idx;
        queryPoint_.setModified();
    }

    if (n < 1)
    {
        return;
    }

    p.x = p.x / static_cast<double>(n);
    p.y = p.y / static_cast<double>(n);
    p.z = p.z / static_cast<double>(n);
    p.elevation = p.elevation / static_cast<double>(n);

    // Append new voxel to voxel array.
    voxels_.push_back(std::move(p));
}

void ComputeSegmentationNNAction::findNearestNeighbor(Point &a)
{
    a.dist = std::numeric_limits<double>::max();
    a.next = SIZE_MAX;

    voxels_.findRadius(a.x,
                       a.y,
                       a.z,
                       parameters_.searchRadiusLeafPoints,
                       search_);

    for (size_t j = 0; j < search_.size(); j++)
    {
        Point &b = voxels_[search_[j]];

        if (b.group != a.group)
        {
            double x = b.x - a.x;
            double y = b.y - a.y;
            double z = b.z - a.z;
            double d = (x * x) + (y * y) + (z * z);

            if (d < a.dist)
            {
                a.dist = d;
                a.next = search_[j];
            }
        }
    }
}

bool ComputeSegmentationNNAction::trunkVoxel(const Point &a)
{
    return a.group == SIZE_MAX &&
           !(a.descriptor < parameters_.woodThresholdMin);
}

void ComputeSegmentationNNAction::startGroup(const Point &a, bool trunk)
{
    if (trunk)
    {
        if (parameters_.zCoordinatesAsElevation)
        {
            groupMinimum_ = a.z;
        }
        else
        {
            groupMinimum_ = a.elevation;
        }
    }

    groupMaximum_ = groupMinimum_;

    group_.nPoints++;
    group_.boundary.extend(a.x, a.y, a.z);
    group_.averagePoint[0] += a.x;
    group_.averagePoint[1] += a.y;
    group_.averagePoint[2] += a.z;
}

void ComputeSegmentationNNAction::continueGroup(const Point &a, bool trunk)
{
    if (trunk)
    {
        if (parameters_.zCoordinatesAsElevation)
        {
            if (a.z < groupMinimum_)
            {
                groupMinimum_ = a.z;
            }
            else if (a.z > groupMaximum_)
            {
                groupMaximum_ = a.z;
            }
        }
        else
        {
            if (a.elevation < groupMinimum_)
            {
                groupMinimum_ = a.elevation;
            }
            else if (a.elevation > groupMaximum_)
            {
                groupMaximum_ = a.elevation;
            }
        }
    }

    group_.nPoints++;
    group_.boundary.extend(a.x, a.y, a.z);
    group_.averagePoint[0] += a.x;
    group_.averagePoint[1] += a.y;
    group_.averagePoint[2] += a.z;
}

void ComputeSegmentationNNAction::mergeToGroup(Group &dst, const Group &src)
{
    dst.nPoints += src.nPoints;
    dst.boundary.extend(src.boundary);
    dst.averagePoint[0] += src.averagePoint[0];
    dst.averagePoint[1] += src.averagePoint[1];
    dst.averagePoint[2] += src.averagePoint[2];
}
