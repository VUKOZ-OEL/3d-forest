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

/** @file SegmentationL1ActionInterface.hpp */

#ifndef SEGMENTATION_L1_ACTION_INTERFACE_HPP
#define SEGMENTATION_L1_ACTION_INTERFACE_HPP

#include <ProgressActionInterface.hpp>
#include <SegmentationL1Context.hpp>

/** Segmentation L1 Action Interface. */
class SegmentationL1ActionInterface : public ProgressActionInterface
{
public:
    SegmentationL1ActionInterface() : ProgressActionInterface(){};
    virtual ~SegmentationL1ActionInterface() = default;

    virtual void initialize(SegmentationL1Context *context) = 0;
    virtual void next() = 0;
};

#endif /* SEGMENTATION_L1_ACTION_INTERFACE_HPP */
