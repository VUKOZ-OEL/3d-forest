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

#include <Thread.hpp>
#include <Vector3.hpp>

/** Segmentation Thread. */
class SegmentationThread : public Thread
{
public:
    SegmentationThread();
    virtual ~SegmentationThread();

    void setup(int voxelSize, int threshold);

    virtual bool compute();

protected:
    /** Segmentation Thread State. */
    enum State
    {
        STATE_NEW,
        STATE_INITIALIZE,
        STATE_VOXEL_SIZE,
        STATE_THRESHOLD,
    };

    bool initialized_;
    State state_;
    int progress_;
    int voxelSize_;
    int threshold_;
};

#endif /* SEGMENTATION_THREAD_HPP */
