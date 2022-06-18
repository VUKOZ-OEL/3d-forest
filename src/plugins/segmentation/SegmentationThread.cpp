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

#include <Editor.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationThread", msg)

SegmentationThread::SegmentationThread(Editor *editor)
    : editor_(editor),
      state_(STATE_NEW),
      initialized_(false),
      progress_(0),
      voxelSize_(0),
      threshold_(0)
{
    LOG_DEBUG_LOCAL("");
}

SegmentationThread::~SegmentationThread()
{
    LOG_DEBUG_LOCAL("");
}

void SegmentationThread::start(int voxelSize, int threshold)
{
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize << "> threshold <" << threshold
                                  << ">");

    cancel();

    // Select state
    bool restart;

    if (state_ == STATE_NEW)
    {
        state_ = STATE_INITIALIZE;
        restart = true;
    }
    else if (voxelSize != voxelSize_)
    {
        state_ = STATE_VOXEL_SIZE;
        restart = true;
    }
    else if (threshold != threshold_)
    {
        state_ = STATE_THRESHOLD;
        restart = true;
    }
    else
    {
        restart = false;
    }

    // Start state
    if (restart)
    {
        LOG_DEBUG_LOCAL("state <" << state_ << ">");

        initialized_ = false;
        progress_ = 0;

        voxelSize_ = voxelSize;
        threshold_ = threshold;

        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("state <" << state_ << ">");

    // Next step
    bool finishedState;
    double timeBegin = getRealTime();

    if (state_ == STATE_INITIALIZE)
    {
        finishedState = computeInitialize();
        if (finishedState)
        {
            state_ = STATE_VOXEL_SIZE;
        }
    }
    else if (state_ == STATE_VOXEL_SIZE)
    {
        finishedState = computeVoxelSize();
        if (finishedState)
        {
            state_ = STATE_THRESHOLD;
        }
    }
    else if (state_ == STATE_THRESHOLD)
    {
        finishedState = computeThreshold();
        if (finishedState)
        {
            state_ = STATE_FINISHED;
        }
    }
    else
    {
        LOG_DEBUG_LOCAL("nothing to do");
        state_ = STATE_FINISHED;
    }

    double timeEnd = getRealTime();
    double msec = (timeEnd - timeBegin) * 1000.;
    LOG_DEBUG_LOCAL("time <" << msec << "> [ms]");

    // Callback
    bool finishedTask;

    if (state_ == STATE_FINISHED)
    {
        finishedTask = true;
    }
    else
    {
        finishedTask = false;
    }

    if (callback_)
    {
        LOG_DEBUG_LOCAL("callback finished <" << finishedTask << ">");
        callback_->threadProgress(finishedTask);
    }

    return finishedTask;
}

bool SegmentationThread::computeInitialize()
{
    LOG_DEBUG_LOCAL("");

    // This state has no progress steps - it can not be canceled
    // Remove all layers, keep default main layer 0
    Layers layers;
    layers.clear();
    layers.setDefault();
    editor_->setLayers(layers);

    // Set all points to layer 0
    Query query(editor_);
    query.selectBox(editor_->clipBoundary());
    query.exec();

    while (query.next())
    {
        query.layer() = 0;
        query.setModified();
    }

    query.flush();

    return true;
}

bool SegmentationThread::computeVoxelSize()
{
    LOG_DEBUG_LOCAL("");
    return true;
}

bool SegmentationThread::computeThreshold()
{
    LOG_DEBUG_LOCAL("");
    return true;
}
