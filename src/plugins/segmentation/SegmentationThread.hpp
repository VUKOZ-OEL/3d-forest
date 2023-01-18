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
#include <SegmentationActionCount.hpp>
#include <SegmentationData.hpp>
#include <SegmentationElements.hpp>
#include <SegmentationMap.hpp>
#include <SegmentationParameters.hpp>
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
    void restart(const SegmentationParameters &parameters);
    virtual bool compute();

    int progressPercent() const;

private:
    Editor *editor_;
    Query query_;

    SegmentationParameters parameters_;
    SegmentationData data_;

    SegmentationActionCount actionCount_;
};

#endif /* SEGMENTATION_THREAD_HPP */
