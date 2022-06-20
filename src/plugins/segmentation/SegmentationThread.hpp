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

    void start(int voxelSize, int threshold);
    int progressPercent() const;

    virtual bool compute();

protected:
    /** Segmentation Thread State. */
    enum State
    {
        STATE_VOXEL_SIZE,
        STATE_THRESHOLD,
        STATE_FINISHED,
    };

    Editor *editor_;

    State state_;
    bool stateInitialized_;
    bool layersCreated_;
    uint64_t progressMax_;
    uint64_t progressValue_;
    int progressPercent_;

    int voxelSize_;
    int threshold_;

    Voxels voxels_;

    void setState(State state);
    void updateProgress(uint64_t increment);
    void computeInitializeLayers();
    bool computeVoxelSize();
    bool computeThreshold();
};

#endif /* SEGMENTATION_THREAD_HPP */
