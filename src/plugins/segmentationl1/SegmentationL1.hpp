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

#include <SegmentationL1ActionCount.hpp>
#include <SegmentationL1ActionRandom.hpp>
#include <SegmentationL1Context.hpp>
class Editor;

/** Segmentation L1. */
class SegmentationL1
{
public:
    static const size_t npos = SIZE_MAX;

    SegmentationL1(Editor *editor);
    ~SegmentationL1();

    void clear();
    bool applyParameters(const SegmentationL1Parameters &parameters);
    bool step();
    int progressPercent() const;

private:
    SegmentationL1Context context_;

    SegmentationL1ActionCount actionCount_;
    SegmentationL1ActionRandom actionRandom_;

    std::vector<SegmentationL1ActionInterface *> actions_;
    size_t currentAction_;

    void initializeCurrentAction();
};

#endif /* SEGMENTATION_L1_HPP */
