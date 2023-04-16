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

/** @file SegmentationL1Context.hpp */

#ifndef SEGMENTATION_L1_CONTEXT_HPP
#define SEGMENTATION_L1_CONTEXT_HPP

#include <Query.hpp>
#include <SegmentationL1Median.hpp>
#include <SegmentationL1Parameters.hpp>
#include <SegmentationL1Pca.hpp>
#include <SegmentationL1Point.hpp>
#include <VoxelFile.hpp>
class Editor;

/** Segmentation L1 Context. */
class SegmentationL1Context
{
public:
    Editor *editor;
    Query query;

    SegmentationL1Parameters parameters;

    uint64_t nPoints;
    std::vector<SegmentationL1Point> samples;
    std::vector<SegmentationL1Point> samplesBackup;

    SegmentationL1Pca pca;
    SegmentationL1Median median;

    SegmentationL1Context(Editor *editor);

    void clear();

    void execInitialSamplesQuery();
};

#endif /* SEGMENTATION_L1_CONTEXT_HPP */
