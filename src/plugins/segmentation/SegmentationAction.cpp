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

/** @file SegmentationAction.cpp */

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Math.hpp>
#include <SegmentationAction.hpp>

#define LOG_MODULE_NAME "SegmentationAction"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define SEGMENTATION_STEP_RESET_POINTS 0
#define SEGMENTATION_STEP_COUNT_POINTS 1
#define SEGMENTATION_STEP_POINTS_TO_VOXELS 2
#define SEGMENTATION_STEP_CREATE_VOXEL_INDEX 3
#define SEGMENTATION_STEP_CREATE_TREES 4
#define SEGMENTATION_STEP_CONNECT_VOXELS 5
#define SEGMENTATION_STEP_CREATE_SEGMENTS 6
#define SEGMENTATION_STEP_VOXELS_TO_POINTS 7

SegmentationAction::SegmentationAction(Editor *editor)
    : editor_(editor),
      query_(editor),
      queryPoint_(editor)
{
    LOG_DEBUG(<< "Create.");
}

SegmentationAction::~SegmentationAction()
{
    LOG_DEBUG(<< "Destroy.");
}

void SegmentationAction::clear()
{
    LOG_DEBUG(<< "Clear.");

    query_.clear();
    queryPoint_.clear();

    voxelSize_ = 0;
    descriptor_ = 0;
    trunkRadius_ = 0;
    leafRadius_ = 0;
    elevationMin_ = 0;
    elevationMax_ = 0;
    treeHeight_ = 0;
    useZ_ = false;
    onlyTrunks_ = false;

    nPointsTotal_ = 0;
    nPointsInFilter_ = 0;

    voxels_.clear();
    groups_.clear();
    path_.clear();
}

void SegmentationAction::start(double voxelSize,
                               double descriptor,
                               double trunkRadius,
                               double leafRadius,
                               double elevationMin,
                               double elevationMax,
                               double treeHeight,
                               bool useZ,
                               bool onlyTrunks)
{
    LOG_DEBUG(<< "Start.");

    // Set input parameters.
    voxelSize_ = voxelSize;
    descriptor_ = descriptor;
    trunkRadius_ = trunkRadius;
    leafRadius_ = leafRadius;
    treeHeight_ = treeHeight;
    useZ_ = useZ;
    onlyTrunks_ = onlyTrunks;

    // Elevation parameters.
    double elevationFilterMin;
    double elevationFilterMax;

    if (useZ_)
    {
        Box<double> extent = editor_->boundary();
        LOG_DEBUG(<< "extent <" << extent << ">.");
        elevationFilterMin = extent.min(2);
        elevationFilterMax = extent.max(2);
    }
    else
    {
        Range<double> elevationRange = editor_->elevationFilter();
        LOG_DEBUG(<< "elevationRange <" << elevationRange << ">.");
        elevationFilterMin = elevationRange.minimumValue();
        elevationFilterMax = elevationRange.maximumValue();
    }

    double elevationDelta = elevationFilterMax - elevationFilterMin;

    elevationMin_ = elevationFilterMin + (elevationMin * elevationDelta);
    elevationMax_ = elevationFilterMin + (elevationMax * elevationDelta);

    LOG_DEBUG(<< "elevationDelta <" << elevationDelta << ">.");
    LOG_DEBUG(<< "elevationMin <" << elevationMin_ << ">.");
    LOG_DEBUG(<< "elevationMax <" << elevationMax_ << ">.");

    // Clear work data.
    nPointsTotal_ = editor_->datasets().nPoints();
    nPointsInFilter_ = 0;

    voxels_.clear();
    groups_.clear();
    path_.clear();

    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setMaximumSteps({4.0, 1.0, 24.0, 1.0, 25.0, 35.0, 1.0, 9.0});
    progress_.setValueSteps(SEGMENTATION_STEP_RESET_POINTS);
}

void SegmentationAction::next()
{
    switch (progress_.valueSteps())
    {
        case SEGMENTATION_STEP_RESET_POINTS:
            stepResetPoints();
            break;

        case SEGMENTATION_STEP_COUNT_POINTS:
            stepCountPoints();
            break;

        case SEGMENTATION_STEP_POINTS_TO_VOXELS:
            stepPointsToVoxels();
            break;

        case SEGMENTATION_STEP_CREATE_VOXEL_INDEX:
            stepCreateVoxelIndex();
            break;

        case SEGMENTATION_STEP_CREATE_TREES:
            stepCreateTrees();
            break;

        case SEGMENTATION_STEP_CONNECT_VOXELS:
            stepConnectVoxels();
            break;

        case SEGMENTATION_STEP_CREATE_SEGMENTS:
            stepCreateSegments();
            break;

        case SEGMENTATION_STEP_VOXELS_TO_POINTS:
            stepVoxelsToPoints();
            break;

        default:
            // empty
            break;
    }
}

void SegmentationAction::stepResetPoints()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        // Initialize. Remove all segments and create default main segment.
        Segments segments;
        segments.setDefault();

        QueryFilterSet segmentsFilter;
        segmentsFilter.clear();
        segmentsFilter.setFilter(0, true);
        segmentsFilter.setFilterEnabled(true);

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

        // Set point segment to main segment.
        query_.segment() = 0;

        query_.setModified();

        progress_.addValueStep(1);
        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(nPointsTotal_, 1000);
    progress_.setValueSteps(SEGMENTATION_STEP_COUNT_POINTS);
}

void SegmentationAction::stepCountPoints()
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

    LOG_DEBUG(<< "Counted <" << nPointsInFilter_ << "> points.");

    query_.reset();

    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(SEGMENTATION_STEP_POINTS_TO_VOXELS);
}

void SegmentationAction::stepPointsToVoxels()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point index to voxel is none:
        if (query_.voxel() == SIZE_MAX && !(query_.elevation() < elevationMin_))
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

    LOG_DEBUG(<< "Created <" << voxels_.size() << "> points.");

    query_.reset();

    progress_.setMaximumStep(voxels_.size(), 100);
    progress_.setValueSteps(SEGMENTATION_STEP_CREATE_VOXEL_INDEX);
}

void SegmentationAction::stepCreateVoxelIndex()
{
    // Create voxel index.
    // TBD: Use some iterative algorithm with increasing progress.
    voxels_.createIndex();

    LOG_DEBUG(<< "Created index.");

    progress_.setMaximumStep(voxels_.size(), 10);
    progress_.setValueSteps(SEGMENTATION_STEP_CREATE_TREES);
}

void SegmentationAction::stepCreateTrees()
{
    progress_.startTimer();

    // If it is the first call, initialize:
    if (progress_.valueStep() == 0)
    {
        // Start from the first voxel.
        pointIndex_ = 0;
        // Set group id to zero.
        groupId_ = 0;
        // Set the path and the group empty.
        path_.resize(0);
        group_.resize(0);
    }

    // Repeat until all voxels and the last path are processed:
    while (pointIndex_ < voxels_.size() || path_.size() > 0)
    {
        // If the path is empty, try to start new path:
        if (path_.empty())
        {
            // If a voxel is not processed and meets
            // criteria (for wood), add it to the path.
            if (isTrunkVoxel(voxels_[pointIndex_]))
            {
                startGroup(voxels_[pointIndex_]);
                voxels_[pointIndex_].group = groupId_;
                path_.push_back(pointIndex_);
            }

            pointIndex_++;
            progress_.addValueStep(1);
        }
        // Else the path is being processed:
        else
        {
            // Add the path to the current group.
            size_t idx = group_.size();
            for (size_t i = 0; i < path_.size(); i++)
            {
                group_.push_back(path_[i]);
            }

            // Set the path empty.
            path_.resize(0);

            // Try to expand the current group with neighbor voxels:
            for (size_t i = idx; i < group_.size(); i++)
            {
                Point &a = voxels_[group_[i]];
                voxels_.findRadius(a.x, a.y, a.z, trunkRadius_, search_);
                for (size_t j = 0; j < search_.size(); j++)
                {
                    Point &b = voxels_[search_[j]];
                    // If a voxel in search radius is not processed and meets
                    // criteria (for wood), add it to group expansion.
                    if (isTrunkVoxel(b))
                    {
                        continueGroup(b);
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
                if (!(groupHeight < treeHeight_) &&
                    groupMinimum_ < elevationMax_)
                {
                    // Mark this group as future segment.
                    groups_[groupId_] = 0;
                    // Increment group id by one.
                    groupId_++;
                }
                // Else throw away the current group:
                else
                {
                    // Set all voxels from the group as not processed.
                    for (size_t i = 0; i < group_.size(); i++)
                    {
                        voxels_[group_[i]].group = SIZE_MAX;
                    }
                }

                // Prepare start of the next group. Set the group empty.
                group_.resize(0);
            }
        }

        if (progress_.timedOut())
        {
            return;
        }
    }

    progress_.setMaximumStep(voxels_.size(), 10);
    progress_.setValueSteps(SEGMENTATION_STEP_CONNECT_VOXELS);

    if (onlyTrunks_)
    {
        progress_.setMaximumStep();
        progress_.setValueSteps(SEGMENTATION_STEP_CREATE_SEGMENTS);
    }
}

void SegmentationAction::stepConnectVoxels()
{
    progress_.startTimer();

    if (progress_.valueStep() == 0)
    {
        // Initialize the current path as finished.
        // Group id is the next unused group id value.
        pointIndex_ = 0;
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
                Point &a = voxels_[pointIndex_];
                a.group = groupId_;
                findNearestNeighbor(a);

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
                // Set the path as finished.
                // (It was not possible to connect this path.)
                path_.resize(0);

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

                    // Set the path as finished.
                    path_.resize(0);

                    // Increment group id for the next path by one.
                    groupId_++;
                }
                // Else nearest neighbor U does not belong to any group,
                // expand the path with new voxel U:
                else
                {
                    // Set group of U to the same group as voxels in the path.
                    a.group = voxels_[path_[0]].group;

                    // Append U into the path.
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

    progress_.setMaximumStep();
    progress_.setValueSteps(SEGMENTATION_STEP_CREATE_SEGMENTS);
}

void SegmentationAction::stepCreateSegments()
{
    // Initialize new segments.
    Segments segments;
    segments.setDefault();

    QueryFilterSet segmentsFilter;
    segmentsFilter.setFilter(0, true);
    segmentsFilter.setFilterEnabled(true);

    Segment segment;
    std::string segmentLabel;
    Vector3<double> segmentColor;

    // For each final group, perform the following:
    size_t segmentId = 1;
    for (auto &it : groups_)
    {
        // Create new segment.
        segmentLabel = "Segment " + std::to_string(segmentId);
        segmentColor =
            ColorPalette::WindowsXp32[segmentId %
                                      ColorPalette::WindowsXp32.size()];
        segment.set(segmentId, segmentLabel, segmentColor);

        // Append new segment to segments.
        segments.push_back(segment);
        segmentsFilter.setFilter(segmentId, true);

        // Set segment id to this group.
        it.second = segmentId;
        segmentId++;
    }

    // Set new segments to editor.
    editor_->setSegments(segments);
    editor_->setSegmentsFilter(segmentsFilter);

    progress_.setMaximumStep(nPointsInFilter_, 1000);
    progress_.setValueSteps(SEGMENTATION_STEP_VOXELS_TO_POINTS);
}

void SegmentationAction::stepVoxelsToPoints()
{
    progress_.startTimer();

    // For each point in filtered datasets:
    while (query_.next())
    {
        // If point belongs to some voxel:
        size_t pointIndex = query_.voxel();
        if (pointIndex < voxels_.size())
        {
            // If voxel's group belongs to a segment:
            size_t groupIndex = voxels_[pointIndex].group;
            if (groups_.count(groupIndex) > 0)
            {
                // Set point segment to the same value as voxel segment.
                query_.segment() = groups_.at(groupIndex);
                query_.setModified();
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

    progress_.setValueStep(progress_.maximumStep());
    progress_.setValueSteps(progress_.maximumSteps());
}

void SegmentationAction::createVoxel()
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
                                  voxelSize_);
    queryPoint_.exec();

    while (queryPoint_.next())
    {
        p.x += queryPoint_.x();
        p.y += queryPoint_.y();
        p.z += queryPoint_.z();
        p.elevation += queryPoint_.elevation();

        if (queryPoint_.descriptor() > p.descriptor)
        {
            p.descriptor = queryPoint_.descriptor();
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

void SegmentationAction::findNearestNeighbor(Point &a)
{
    a.dist = std::numeric_limits<double>::max();
    a.next = SIZE_MAX;

    voxels_.findRadius(a.x, a.y, a.z, leafRadius_, search_);

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

bool SegmentationAction::isTrunkVoxel(const Point &a)
{
    return a.group == SIZE_MAX && !(a.descriptor < descriptor_);
}

void SegmentationAction::startGroup(const Point &a)
{
    if (useZ_)
    {
        groupMinimum_ = a.z;
    }
    else
    {
        groupMinimum_ = a.elevation;
    }

    groupMaximum_ = groupMinimum_;
}

void SegmentationAction::continueGroup(const Point &a)
{
    if (useZ_)
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
