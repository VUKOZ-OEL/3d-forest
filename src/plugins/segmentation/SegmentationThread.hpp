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

/** @file SegmentationThread.hpp */

#ifndef SEGMENTATION_THREAD_HPP
#define SEGMENTATION_THREAD_HPP

#include <Query.hpp>
#include <SegmentationElements.hpp>
#include <SegmentationMap.hpp>
#include <Thread.hpp>
#include <Vector3.hpp>
#include <Voxels.hpp>

class Editor;

/** Segmentation Thread. */
class SegmentationThread : public Thread
{
public:
    SegmentationThread(Editor *editor);
    virtual ~SegmentationThread();

    void clear();

    void start(int voxelSize,
               int seedElevationMinimumPercent,
               int seedElevationMaximumPercent,
               int treeHeightMinimumPercent,
               int searchRadius,
               int neighborPoints);

    int progressPercent() const { return progressPercent_; }

    virtual bool compute();

    const Editor *editor() const { return editor_; }
    const SegmentationMap &segmentationMap() const { return segmentationMap_; }

private:
    /** Segmentation Thread State. */
    enum State
    {
        STATE_INITIALIZE_VOXELS,
        STATE_CREATE_VOXELS,
        STATE_SORT_VOXELS,
        STATE_PROCESS_VOXELS,
        STATE_INITIALIZE_ELEMENTS,
        STATE_CREATE_ELEMENTS,
        STATE_MERGE_ELEMENTS,
        STATE_CREATE_LAYERS,
        STATE_FINISHED,
    };

    Editor *editor_;
    Query query_;

    State state_;
    bool stateInitialized_;
    bool layersCreated_;
    int progressCounter_;
    int progressPercent_;
    uint64_t progressMax_;
    uint64_t progressValue_;
    double stateTime_;

    int voxelSize_;
    int seedElevationMinimumPercent_;
    int seedElevationMaximumPercent_;
    int treeHeightMinimumPercent_;
    int searchRadius_;
    int neighborPoints_;

    double seedElevationMinimum_;
    double seedElevationMaximum_;
    double treeHeightMinimum_;

    double timeBegin;
    double timeNow;
    double timeElapsed;

    Voxels voxels_;
    SegmentationElements elements_;
    SegmentationMap segmentationMap_;

    void setState(State state);
    void updateProgressPercent();
    bool hasTimedout(int interleave = 10);

    void resetLayers();

    bool computeInitializeVoxels();
    bool computeCreateVoxels();
    bool computeSortVoxels();
    bool computeProcessVoxels();
    bool computeInitializeElements();
    bool computeCreateElements();
    bool computeMergeElements();
    bool computeCreateLayers();
};

#endif /* SEGMENTATION_THREAD_HPP */
