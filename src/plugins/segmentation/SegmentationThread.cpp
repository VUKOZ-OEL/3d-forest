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
#include <SegmentationPca.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationThread", msg)

SegmentationThread::SegmentationThread(Editor *editor)
    : editor_(editor),
      query_(editor_),
      state_(STATE_FINISHED),
      stateInitialized_(false),
      layersCreated_(true),
      progressMax_(0),
      progressValue_(0),
      progressPercent_(0),
      voxelSize_(0),
      thresholdPercent_(0),
      thresholdIntensity_(0)
{
    LOG_DEBUG_LOCAL("");
}

SegmentationThread::~SegmentationThread()
{
    LOG_DEBUG_LOCAL("");
}

void SegmentationThread::clear()
{
    query_.clear();
    voxels_.clear();
    pca_.clear();
}

void SegmentationThread::start(int voxelSize, int thresholdPercent)
{
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize << "> threshold <"
                                  << thresholdPercent << ">");

    // Cancel current computation
    cancel();

    // Select state to start from
    State startState;

    if (voxelSize != voxelSize_)
    {
        startState = STATE_VOXEL_SIZE;
    }
    else if (thresholdPercent != thresholdPercent_)
    {
        startState = STATE_THRESHOLD;
    }
    else
    {
        startState = STATE_FINISHED;
    }

    // Start selected state
    if (startState != STATE_FINISHED)
    {
        setState(startState);

        voxelSize_ = voxelSize;
        thresholdPercent_ = thresholdPercent;

        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("state <" << state_ << ">");

    // Next step
    double timeBegin = getRealTime();

    if (layersCreated_ && !stateInitialized_)
    {
        computeInitializeLayers();
        layersCreated_ = false;
    }

    if (state_ == STATE_VOXEL_SIZE)
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
            setState(STATE_MERGE_CLUSTERS);
        }
    }
    else if (state_ == STATE_MERGE_CLUSTERS)
    {
        bool finishedState = computeMergeClusters();
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
    (void)msec;

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

void SegmentationThread::updateProgressPercent()
{
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

void SegmentationThread::computeInitializeLayers()
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
        return;
    }

    // Set all points to layer 0
    query_.selectBox(editor_->clipBoundary());
    query_.exec();

    while (query_.next())
    {
        query_.layer() = 0;
        query_.setModified();
    }

    query_.flush();
}

bool SegmentationThread::computeVoxelSize()
{
    LOG_DEBUG_LOCAL("");

    double timeBegin = getRealTime();

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

        voxels_.create(editor_->clipBoundary(), voxelSize_);

        progressMax_ = voxels_.size();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    Box<double> cell;
    size_t index;
    size_t counter = 0;

    while (voxels_.next(&cell, &index))
    {
        // Compute one voxel
        pca_.compute(query_, voxels_, cell, index);

        // Update progress
        progressValue_++;
        counter++;
        if (counter > 10)
        {
            counter = 0;
            double timeNow = getRealTime();
            if (timeNow - timeBegin > 0.1)
            {
                updateProgressPercent();
                return false;
            }
        }
    }

    // Finished
    progressPercent_ = 100;

    editor_->setVoxels(voxels_);

    return true;
}

bool SegmentationThread::computeThreshold()
{
    float min = pca_.intensityMin();
    float max = pca_.intensityMax();
    float thresholdNormalized = static_cast<float>(thresholdPercent_) * 0.01F;
    thresholdIntensity_ = min + ((max - min) * thresholdNormalized);
    LOG_DEBUG_LOCAL("threshold <" << thresholdIntensity_ << "> from min <"
                                  << min << "> max <" << max << "> percent <"
                                  << thresholdPercent_ << ">");
    progressPercent_ = 100;
    return true;
}

bool SegmentationThread::computeMergeClusters()
{
    LOG_DEBUG_LOCAL("");

    double timeBegin = getRealTime();

    // Initialization
    if (!stateInitialized_)
    {
        progressMax_ = voxels_.size();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step

    progressPercent_ = 100;

    return true;
}
