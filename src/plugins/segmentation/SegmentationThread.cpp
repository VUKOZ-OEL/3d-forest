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
      stateInitialized_(false),
      progressMax_(0),
      progressValue_(0),
      progressPercent_(0),
      voxelSize_(0),
      threshold_(0),
      voxel_(editor_)
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

    // Cancel current computation
    cancel();

    // Select state to start from
    State startState = STATE_FINISHED;

    if (state_ == STATE_NEW)
    {
        startState = STATE_INITIALIZE;
    }
    else if (voxelSize != voxelSize_)
    {
        startState = STATE_VOXEL_SIZE;
    }
    else if (threshold != threshold_)
    {
        startState = STATE_THRESHOLD;
    }

    // Start selected state
    if (startState != STATE_FINISHED)
    {
        setState(startState);

        voxelSize_ = voxelSize;
        threshold_ = threshold;

        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("state <" << state_ << ">");

    // Next step
    double timeBegin = getRealTime();

    if (state_ == STATE_INITIALIZE)
    {
        bool finishedState = computeInitialize();
        if (finishedState)
        {
            setState(STATE_VOXEL_SIZE);
        }
    }
    else if (state_ == STATE_VOXEL_SIZE)
    {
        bool finishedState = computeVoxelSize();
        if (finishedState)
        {
            setState(STATE_THRESHOLD);
        }
    }
    else if (state_ == STATE_THRESHOLD)
    {
        bool finishedState = computeThreshold();
        if (finishedState)
        {
            setState(STATE_FINISHED);
        }
    }
    else
    {
        LOG_DEBUG_LOCAL("nothing to do");
        setState(STATE_FINISHED);
    }

    double timeEnd = getRealTime();
    double msec = (timeEnd - timeBegin) * 1000.;
    LOG_DEBUG_LOCAL("time <" << msec << "> [ms]");

    // Check if the whole task is finished and call callback
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

void SegmentationThread::setState(State state)
{
    LOG_DEBUG_LOCAL("state <" << state << ">");
    state_ = state;
    stateInitialized_ = false;
    progressMax_ = 0;
    progressValue_ = 0;
    progressPercent_ = 0;
}

void SegmentationThread::updateProgress(uint64_t increment)
{
    progressValue_ += increment;

    if (progressMax_ > 0)
    {
        const double h = static_cast<double>(progressMax_);
        const double v = static_cast<double>(progressValue_);
        const double r = v / h;
        progressPercent_ = static_cast<int>(r * 100.);
    }
    else
    {
        progressPercent_ = 100;
    }
}

int SegmentationThread::progressPercent() const
{
    return progressPercent_;
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

    const Datasets &datasets = editor_->datasets();
    LOG_DEBUG_LOCAL("number of points <" << datasets.nPoints() << ">");
    if (datasets.nPoints() < 1)
    {
        return true;
    }

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

    // Initialization
    if (!stateInitialized_)
    {
        voxels_.clear();

        const Datasets &datasets = editor_->datasets();
        LOG_DEBUG_LOCAL("number of points <" << datasets.nPoints() << ">");
        if (datasets.nPoints() < 1)
        {
            return true;
        }

        voxel_.setGrid(1000);
        progressMax_ = voxel_.gridSize();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    if (voxel_.nextGrid())
    {
        updateProgress(1);


        return false;
    }

    // Finished
    progressPercent_ = 100;

    editor_->setVoxels(voxels_);

    return true;
}

bool SegmentationThread::computeThreshold()
{
    LOG_DEBUG_LOCAL("");
    return true;
}
