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
#include <SegmentationElement.hpp>
#include <SegmentationPca.hpp>
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
               int descriptorThresholdPercent,
               int minimumVoxelsInElement);

    int progressPercent() const { return progressPercent_; }

    virtual bool compute();

    const Editor *editor() const { return editor_; }

protected:
    /** Segmentation Thread State. */
    enum State
    {
        STATE_INITIALIZE_VOXELS,
        STATE_CREATE_VOXELS,
        STATE_NORMALIZE_VOXELS,
        STATE_DESCRIPTOR_THRESHOLD,
        STATE_INITIALIZE_ELEMENTS,
        STATE_CREATE_ELEMENTS,
        STATE_MERGE_CLUSTERS,
        STATE_CREATE_LAYERS,
        STATE_FINISHED,
    };

    Editor *editor_;
    Query query_;
    Query queryDescriptor_;

    State state_;
    bool stateInitialized_;
    bool layersCreated_;
    uint64_t progressMax_;
    uint64_t progressValue_;
    int progressPercent_;

    int voxelSize_;
    int minimumVoxelsInElement_;
    int descriptorThresholdPercent_;
    float descriptorThresholdNormalized_;

    SegmentationPca pca_;
    SegmentationElement elements_;

    Voxels voxels_;

    void setState(State state);
    void updateProgressPercent();

    void resetLayers();

    bool computeInitializeVoxels();
    bool computeCreateVoxels();
    bool computeNormalizeVoxels();
    bool computeDescriptorThreshold();
    bool computeInitializeElements();
    bool computeCreateElements();
    bool computeMergeClusters();
    bool computeCreateLayers();
};

#endif /* SEGMENTATION_THREAD_HPP */
