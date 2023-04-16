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

/** @file SegmentationL1.hpp */

#ifndef SEGMENTATION_L1_HPP
#define SEGMENTATION_L1_HPP

#include <SegmentationL1Context.hpp>
#include <SegmentationL1TaskFilter.hpp>
#include <SegmentationL1TaskFinish.hpp>
#include <SegmentationL1TaskMedian.hpp>
#include <SegmentationL1TaskPca.hpp>
#include <SegmentationL1TaskSample.hpp>
class Editor;

/** Segmentation L1. */
class SegmentationL1
{
public:
    static const size_t npos = SIZE_MAX;

    SegmentationL1(Editor *editor);
    ~SegmentationL1();

    void clear();
    bool applyParameters(const SegmentationL1Parameters &parameters,
                         bool autoStart);
    bool next();
    void progress(size_t &nTasks, size_t &iTask, double &percent) const;

    const SegmentationL1Context &context() const { return context_; }

private:
    SegmentationL1Context context_;

    SegmentationL1TaskFilter taskFilter_;
    SegmentationL1TaskSample taskSample_;
    SegmentationL1TaskPca taskPca_;
    SegmentationL1TaskMedian taskMedian_;
    SegmentationL1TaskFinish taskFinish_;

    std::vector<SegmentationL1TaskInterface *> tasks_;
    size_t currentAction_;

    void initializeCurrentAction();
};

#endif /* SEGMENTATION_L1_HPP */
