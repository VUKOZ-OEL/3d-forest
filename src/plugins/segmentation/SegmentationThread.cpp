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

/** @file SegmentationThread.cpp */

#include <Log.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationThread", msg)

SegmentationThread::SegmentationThread()
    : initialized_(false),
      state_(STATE_NEW)
{
    LOG_DEBUG_LOCAL("");
}

SegmentationThread::~SegmentationThread()
{
    LOG_DEBUG_LOCAL("");
}

void SegmentationThread::setup(int voxelSize, int threshold)
{
    LOG_DEBUG_LOCAL("");

    cancel();

    bool restart;

    if (voxelSize != voxelSize_)
    {
        voxelSize_ = voxelSize;
        state_ = STATE_VOXEL_SIZE;
        restart = true;
    }
    else if (threshold != threshold_)
    {
        threshold_ = threshold;
        state_ = STATE_THRESHOLD;
        restart = true;
    }
    else
    {
        restart = false;
    }

    if (restart)
    {
        LOG_DEBUG_LOCAL("state <" << state_ << ">");

        initialized_ = false;
        progress_ = 0;

        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("initialized <" << initialized_ << ">");

    if (!initialized_)
    {
        initialized_ = true;
        return false;
    }

    LOG_DEBUG_LOCAL("nextState");
    double timeBegin = getRealTime();
    bool finished;

    finished = true;

    double timeEnd = getRealTime();
    double msec = (timeEnd - timeBegin) * 1000.;

    if (callback_)
    {
        LOG_DEBUG_LOCAL("callback finished <" << finished << "> time <"
                                              << msec << ">");
        callback_->threadProgress(finished);
    }

    return finished;
}
