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

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define MODULE_NAME "SegmentationThread"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)
#define LOG_DEBUG_LOCAL_STATE(msg)
//#define LOG_DEBUG_LOCAL_STATE(msg) LOG_MODULE(MODULE_NAME, msg)

SegmentationThread::SegmentationThread(Editor *editor)
    : editor_(editor),
      query_(editor_),
      state_(STATE_FINISHED),
      stateInitialized_(false),
      layersCreated_(true),
      progressCounter_(0),
      progressPercent_(0),
      progressMax_(0),
      progressValue_(0),
      voxelSize_(0),
      seedElevationMinimumPercent_(0),
      seedElevationMaximumPercent_(0),
      treeHeightMinimumPercent_(0),
      seedElevationMinimum_(0),
      seedElevationMaximum_(0),
      treeHeightMinimum_(0),
      timeBegin(0),
      timeNow(0),
      timeElapsed(0)
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
}

void SegmentationThread::start(int voxelSize,
                               int seedElevationMinimumPercent,
                               int seedElevationMaximumPercent,
                               int treeHeightMinimumPercent)
{
    // clang-format off
    LOG_DEBUG_LOCAL(
        << "voxelSize <" << voxelSize
        << "> seedElevationMinimumPercent <" << seedElevationMinimumPercent
        << "> seedElevationMaximumPercent <" << seedElevationMaximumPercent
        << "> treeHeightMinimumPercent <" << treeHeightMinimumPercent << ">");
    // clang-format on

    // Cancel current computation
    cancel();

    // Select state to start from
    State startState = STATE_FINISHED;

    if (treeHeightMinimumPercent != treeHeightMinimumPercent_)
    {
        treeHeightMinimumPercent_ = treeHeightMinimumPercent;
        double h = editor_->clipBoundary().length(2);
        treeHeightMinimum_ = h * (treeHeightMinimumPercent_ / 100.0);
        startState = STATE_SORT_VOXELS;
    }

    if (seedElevationMinimumPercent != seedElevationMinimumPercent_ ||
        seedElevationMaximumPercent != seedElevationMaximumPercent_)
    {
        seedElevationMinimumPercent_ = seedElevationMinimumPercent;
        seedElevationMaximumPercent_ = seedElevationMaximumPercent;

        double h = editor_->clipBoundary().length(2);
        seedElevationMinimum_ = h * (seedElevationMinimumPercent_ / 100.0);
        seedElevationMaximum_ = h * (seedElevationMaximumPercent_ / 100.0);

        startState = STATE_SORT_VOXELS;
    }

    if (voxelSize != voxelSize_)
    {
        voxelSize_ = voxelSize;
        startState = STATE_INITIALIZE_VOXELS;
    }

    // Start selected state
    if (startState != STATE_FINISHED)
    {
        setState(startState);
        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("state <" << state_ << ">");

    // Next step
    timeBegin = getRealTime();

    resetLayers();

    if (state_ == STATE_INITIALIZE_VOXELS)
    {
        bool finishedState = computeInitializeVoxels();
        if (finishedState)
        {
            setState(STATE_CREATE_VOXELS);
        }
    }
    else if (state_ == STATE_CREATE_VOXELS)
    {
        bool finishedState = computeCreateVoxels();
        if (finishedState)
        {
            setState(STATE_SORT_VOXELS);
        }
    }
    else if (state_ == STATE_SORT_VOXELS)
    {
        bool finishedState = computeSortVoxels();
        if (finishedState)
        {
            setState(STATE_INITIALIZE_ELEMENTS);
        }
    }
    else if (state_ == STATE_INITIALIZE_ELEMENTS)
    {
        bool finishedState = computeInitializeElements();
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
            setState(STATE_MERGE_ELEMENTS);
        }
    }
    else if (state_ == STATE_MERGE_ELEMENTS)
    {
        bool finishedState = computeMergeElements();
        if (finishedState)
        {
            setState(STATE_CREATE_LAYERS);
        }
    }
    else if (state_ == STATE_CREATE_LAYERS)
    {
        bool finishedState = computeCreateLayers();
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

    timeElapsed = getRealTime() - timeBegin;
    LOG_DEBUG_LOCAL("time <" << (timeElapsed * 1000.) << "> [ms]");

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
    progressCounter_ = 0;
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

bool SegmentationThread::hasTimedout(int interleave)
{
    progressCounter_++;
    if (progressCounter_ >= interleave)
    {
        progressCounter_ = 0;
        timeNow = getRealTime();
        if (timeNow - timeBegin > 0.5)
        {
            return true; // timed out
        }
    }

    return false;
}

void SegmentationThread::resetLayers()
{
    LOG_DEBUG_LOCAL("");

    if (stateInitialized_ || !layersCreated_)
    {
        return;
    }

    // Remove all layers, keep default main layer 0
    Layers layers;
    layers.clear();
    layers.setDefault();
    editor_->setLayers(layers);

    layersCreated_ = false;
}

bool SegmentationThread::computeInitializeVoxels()
{
    LOG_DEBUG_LOCAL("");

    // Initialization
    if (!stateInitialized_)
    {
        const Datasets &datasets = editor_->datasets();
        LOG_DEBUG_LOCAL("number of points <" << datasets.nPoints() << ">");
        if (datasets.nPoints() < 1)
        {
            return true;
        }

        query_.selectBox(editor_->clipBoundary());
        query_.exec();

        progressMax_ = datasets.nPoints();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    while (query_.next())
    {
        query_.layer() = 0;
        query_.descriptor() = 0;
        query_.value() = Voxels::npos;
        query_.setModified();

        // Update progress
        progressValue_++;
        if (hasTimedout(1000))
        {
            updateProgressPercent();
            return false;
        }
    }

    // Finished
    query_.flush();
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeCreateVoxels()
{
    LOG_DEBUG_LOCAL("");

    // Initialize voxels.
    if (!stateInitialized_)
    {
        voxels_.clear();
        voxels_.create(editor_->clipBoundary(), voxelSize_);

        progressMax_ = voxels_.indexSize();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step: iterate over all voxels and compute their descriptors.
    Box<double> cell;
    Voxel voxel;

    while (voxels_.next(&voxel, &cell, &query_))
    {
        size_t nPoints = 0;
        size_t voxelIndex = voxels_.size();

        double meanX = 0;
        double meanY = 0;
        double meanZ = 0;
        double meanElevation = 0;

        // Add reference to voxel item to each point inside this voxel.
        query_.selectBox(cell);
        query_.exec();
        while (query_.next())
        {
            meanX += query_.x();
            meanY += query_.y();
            meanZ += query_.z();
            meanElevation += query_.elevation();

            query_.value() = voxelIndex;
            query_.setModified();

            nPoints++;
        }

        if (nPoints > 0)
        {
            const double d = static_cast<double>(nPoints);
            meanX = meanX / d;
            meanY = meanY / d;
            meanZ = meanZ / d;
            meanElevation = meanElevation / d;
        }

        // Create new occupied voxel item.
        if (nPoints > 0)
        {
            voxel.meanX_ = meanX;
            voxel.meanY_ = meanY;
            voxel.meanZ_ = meanZ;
            voxel.meanElevation_ = meanElevation;
            voxels_.addVoxel(voxel);
        }

        // Update progress.
        progressValue_ = voxels_.visitedVoxelsCount();
        if (progressValue_ == progressMax_)
        {
            LOG_DEBUG_LOCAL_STATE("visitedVoxelsCount");
            break;
        }

        if (hasTimedout(10))
        {
            // Reached maximum time per one compute step.
            // Return and continue later in the next call.
            updateProgressPercent();
            return false;
        }
    }

    // Finished
    query_.flush();
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeSortVoxels()
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
    voxels_.sort(seedElevationMinimum_, seedElevationMaximum_);

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeInitializeElements()
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
        voxels_.at(i).clearState();
    }

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeCreateElements()
{
    LOG_DEBUG_LOCAL("");

    // Initialization
    if (!stateInitialized_)
    {
        progressMax_ = voxels_.sortedSize();
        progressValue_ = 0;
        stateInitialized_ = true;
    }

    // Next step
    for (size_t i = 0; i < voxels_.sortedSize(); i++)
    {
        uint32_t elementIndex;

        elementIndex = elements_.computeBase(voxels_, i, treeHeightMinimum_);

        if (elementIndex != SegmentationElements::npos)
        {
            const SegmentationElement &element = elements_[elementIndex];
            const std::vector<size_t> &voxelList = element.voxelList();
            const size_t nVoxels = voxelList.size();

            LOG_DEBUG_LOCAL("number of voxels in element <"
                            << elementIndex << "> is <" << nVoxels << ">");

            for (size_t j = 0; j < nVoxels; j++)
            {
                Voxel &voxel = voxels_.at(voxelList[j]);
                voxel.elementIndex_ = elementIndex;
            }
        }

        progressValue_++;
        if (hasTimedout(1))
        {
            // Reached maximum time per one compute step.
            // Return and continue later in the next call.
            updateProgressPercent();
            return false;
        }
    }

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeMergeElements()
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

    // Finished
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeCreateLayers()
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
    Box<double> cell;

    // Query
    query_.selectBox(editor_->clipBoundary());
    query_.exec();

    while (query_.next())
    {
        size_t index = query_.value();

        if (index != Voxels::npos)
        {
            Voxel &voxel = voxels_.at(index);

            if (voxel.elementIndex_ != Voxel::npos)
            {
                query_.layer() =
                    voxel.elementIndex_ + 1; // main layer is id = 0
            }

            query_.descriptor() = voxel.descriptor_;
            query_.setModified();
        }
    }

    query_.flush();

    // Layers
    Layers layers;
    layers.setDefault();

    size_t nLayers = elements_.size();
    if (nLayers > 0)
    {
        LOG_DEBUG_LOCAL("number of layers <" << nLayers << ">");

        const std::vector<Vector3<float>> &pal = ColorPalette::WindowsXp32;
        Layer layer;

        for (size_t i = 0; i < nLayers; i++)
        {
            size_t id = i + 1; // main layer is id = 0
            layer.set(id, "Layer " + std::to_string(id), true, pal[i % 32]);
            layers.push_back(layer);
        }
    }

    layersCreated_ = true;

    // Update
    editor_->setVoxels(voxels_);
    editor_->setLayers(layers);

    // Finished
    progressPercent_ = 100;

    return true;
}
