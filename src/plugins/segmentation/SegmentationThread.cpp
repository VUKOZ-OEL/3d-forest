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
#include <SegmentationPca.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define MODULE_NAME "SegmentationThread"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

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
      descriptorThresholdPercent_(0),
      descriptorThresholdNormalized_(0)
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
                               int descriptorThresholdPercent,
                               int minimumVoxelsInElement)
{
    LOG_DEBUG_LOCAL("voxelSize <" << voxelSize << "> threshold <"
                                  << descriptorThresholdPercent << ">");

    // Cancel current computation
    cancel();

    // Select state to start from
    State startState;

    if (voxelSize != voxelSize_)
    {
        startState = STATE_INITIALIZE_VOXELS;
    }
    else if (descriptorThresholdPercent != descriptorThresholdPercent_)
    {
        startState = STATE_DESCRIPTOR_THRESHOLD;
    }
    else if (minimumVoxelsInElement != minimumVoxelsInElement_)
    {
        startState = STATE_INITIALIZE_ELEMENTS;
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
        descriptorThresholdPercent_ = descriptorThresholdPercent;
        minimumVoxelsInElement_ = minimumVoxelsInElement;

        Thread::start();
    }
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("state <" << state_ << ">");

    // Next step
    double timeBegin = getRealTime();

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
            setState(STATE_NORMALIZE_VOXELS);
        }
    }
    else if (state_ == STATE_NORMALIZE_VOXELS)
    {
        bool finishedState = computeNormalizeVoxels();
        if (finishedState)
        {
            setState(STATE_DESCRIPTOR_THRESHOLD);
        }
    }
    else if (state_ == STATE_DESCRIPTOR_THRESHOLD)
    {
        bool finishedState = computeDescriptorThreshold();
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
            setState(STATE_MERGE_CLUSTERS);
        }
    }
    else if (state_ == STATE_MERGE_CLUSTERS)
    {
        bool finishedState = computeMergeClusters();
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

    double timeBegin = getRealTime();

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
    size_t counter = 0;

    while (query_.next())
    {
        query_.layer() = 0;
        query_.descriptor() = 0;
        query_.value() = Voxels::npos;
        query_.setModified();

        // Update progress
        progressValue_++;
        counter++;
        if (counter > 1000)
        {
            counter = 0;
            double timeNow = getRealTime();
            if (timeNow - timeBegin > 0.5)
            {
                updateProgressPercent();
                return false;
            }
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

    double timeBegin = getRealTime();

    // Initialization
    if (!stateInitialized_)
    {
        voxels_.clear();
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
        pca_.computeDescriptor(cell, query_, voxels_, voxel);

        // Update progress
        progressValue_++;
        counter++;
        if (counter > 10)
        {
            counter = 0;
            double timeNow = getRealTime();
            if (timeNow - timeBegin > 0.5)
            {
                updateProgressPercent();
                return false;
            }
        }
    }

    // Out
    //   Point
    //     has value to voxel index
    //   Voxel
    //     computed descriptor

    // Finished
    query_.flush();
    progressPercent_ = 100;

    return true;
}

bool SegmentationThread::computeNormalizeVoxels()
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

bool SegmentationThread::computeDescriptorThreshold()
{
    descriptorThresholdNormalized_ =
        static_cast<float>(descriptorThresholdPercent_) * 0.01F;
    LOG_DEBUG_LOCAL("threshold <" << descriptorThresholdNormalized_
                                  << "> from min <" << voxels_.intensityMin()
                                  << "> max <" << voxels_.intensityMax()
                                  << "> percent <"
                                  << descriptorThresholdPercent_ << ">");
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
        Voxel &voxel = voxels_.at(i);

        voxel.status_ = 0;
        voxel.elementId_ = SegmentationElement::npos;
        voxel.clusterId_ = SegmentationElement::npos;

        // if (voxel.intensity_ < descriptorThresholdNormalized_)
        // {
        //     voxel.status_ |= Voxel::STATUS_IGNORED;
        // }
    }

    // Finished
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
            uint32_t elementId = elements_.elementId();

            LOG_DEBUG_LOCAL("number of voxels in element <"
                            << elementId << "> is <" << nVoxels << ">");

            for (size_t j = 0; j < nVoxels; j++)
            {
                Voxel &voxel = voxels_.at(voxelList[j]);
                voxel.elementId_ = elementId;
            }

            elements_.elementIdNext();
        }
    }

    progressPercent_ = 100;

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

        if (index < Voxels::npos)
        {
            Voxel &voxel = voxels_.at(index);
            if (voxel.elementId_ != SegmentationElement::npos)
            {
                // query_.layer() = voxel.elementId_ + 1;
            }
            query_.descriptor() = voxel.intensity_;
            query_.setModified();
        }
    }

    query_.flush();

    // Layers
    Layers layers;
    layers.setDefault();

    size_t nLayers = elements_.elementId();
    if (nLayers > 1)
    {
        nLayers--; // ignore main layer
        LOG_DEBUG_LOCAL("number of layers <" << nLayers << ">");

        const std::vector<Vector3<float>> &pal = ColorPalette::WindowsXp32;
        Layer layer;

        for (size_t i = 0; i < nLayers; i++)
        {
            size_t id = i + 1;
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
