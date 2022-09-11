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

//#define LOG_DEBUG_LOCAL(msg)
#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationThread", msg)

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
      minimumVoxelsInElement_(0),
      thresholdIntensityPercent_(0),
      thresholdIntensityNormalized_(0)
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

void SegmentationThread::start(int voxelSize,
                               int thresholdIntensityPercent,
                               int minimumVoxelsInElement)
{
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize << "> threshold <"
                                  << thresholdIntensityPercent << ">");

    // Cancel current computation
    cancel();

    // Select state to start from
    State startState;

    if (voxelSize != voxelSize_)
    {
        startState = STATE_VOXEL_SIZE;
    }
    else if (thresholdIntensityPercent != thresholdIntensityPercent_)
    {
        startState = STATE_THRESHOLD_INTENSITY;
    }
    else if (minimumVoxelsInElement != minimumVoxelsInElement_)
    {
        startState = STATE_CREATE_ELEMENTS_START;
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
        thresholdIntensityPercent_ = thresholdIntensityPercent;
        minimumVoxelsInElement_ = minimumVoxelsInElement;

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
            setState(STATE_VOXEL_NORMALIZE);
        }
    }
    else if (state_ == STATE_VOXEL_NORMALIZE)
    {
        bool finishedState = computeVoxelNormalize();
        if (finishedState)
        {
            setState(STATE_THRESHOLD_INTENSITY);
        }
    }
    else if (state_ == STATE_THRESHOLD_INTENSITY)
    {
        bool finishedState = computeThresholdIntensity();
        if (finishedState)
        {
            setState(STATE_CREATE_ELEMENTS_START);
        }
    }
    else if (state_ == STATE_CREATE_ELEMENTS_START)
    {
        bool finishedState = computeCreateElementsStart();
        if (finishedState)
        {
            setState(STATE_CREATE_ELEMENTS);
        }
    }
    else if (state_ == STATE_CREATE_ELEMENTS)
    {
        bool finishedState = computeCreateElements();
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

        progressMax_ = voxels_.indexSize();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    Voxel voxel;
    Box<double> cell;
    size_t counter = 0;

    while (voxels_.next(&voxel, &cell))
    {
        // Compute one voxel [meanX, meanY, meanZ, intensity, density]
        bool computed;
        computed = pca_.compute(&query_, &voxel, cell);
        if (computed)
        {
            voxels_.append(voxel);
        }

        // Update progress
        progressValue_++;
        counter++;
        if (counter > 10)
        {
            counter = 0;
            double timeNow = getRealTime();
            if (timeNow - timeBegin > 1.0) // 0.1)
            {
                updateProgressPercent();
                return false;
            }
        }
    }

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeVoxelNormalize()
{
    LOG_DEBUG_LOCAL("");

    // Initialization
    if (!stateInitialized_)
    {
        progressMax_ = voxels_.size();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    for (size_t i = 0; i < voxels_.size(); i++)
    {
        voxels_.normalize(&voxels_.at(i));
    }

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeThresholdIntensity()
{
    thresholdIntensityNormalized_ =
        static_cast<float>(thresholdIntensityPercent_) * 0.01F;
    LOG_DEBUG_LOCAL("threshold <" << thresholdIntensityNormalized_
                                  << "> from min <" << voxels_.intensityMin()
                                  << "> max <" << voxels_.intensityMax()
                                  << "> percent <" << thresholdIntensityPercent_
                                  << ">");
    progressPercent_ = 100;
    return true;
}

bool SegmentationThread::computeCreateElementsStart()
{
    LOG_DEBUG_LOCAL("");

    // Initialization
    if (!stateInitialized_)
    {
        elements_.clear();

        progressMax_ = voxels_.size();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    for (size_t i = 0; i < voxels_.size(); i++)
    {
        Voxel &voxel = voxels_.at(i);

        voxel.clearState();

        if (voxel.intensity_ < thresholdIntensityNormalized_)
        {
            voxel.status_ |= Voxel::STATUS_IGNORED;
        }
    }

    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeCreateElements()
{
    LOG_DEBUG_LOCAL("");

    // double timeBegin = getRealTime();

    // Initialization
    if (!stateInitialized_)
    {
        progressMax_ = voxels_.size();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    for (size_t i = 0; i < voxels_.size(); i++)
    {
        elements_.computeStart(i, voxels_);
        (void)elements_.compute(&voxels_);

        const std::vector<size_t> &voxelList = elements_.voxelList();
        size_t nVoxels = voxelList.size();

        if (static_cast<int>(nVoxels) >= minimumVoxelsInElement_)
        {
            for (size_t j = 0; j < nVoxels; j++)
            {
                Voxel &voxel = voxels_.at(voxelList[j]);
                voxel.elementId_ = elements_.elementId();
            }

            elements_.elementIdNext();
        }
    }

    progressPercent_ = 100;

    editor_->setVoxels(voxels_);

    return true;
}

bool SegmentationThread::computeMergeClusters()
{
    LOG_DEBUG_LOCAL("");

    // double timeBegin = getRealTime();

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
