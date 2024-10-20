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

/** @file ComputeDescriptorAction.hpp */

#ifndef COMPUTE_DESCRIPTOR_ACTION_HPP
#define COMPUTE_DESCRIPTOR_ACTION_HPP

// Include 3D Forest.
#include <ComputeDescriptorParameters.hpp>
#include <ComputeDescriptorPca.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;

/** Compute Descriptor Action. */
class ComputeDescriptorAction : public ProgressActionInterface
{
public:
    ComputeDescriptorAction(Editor *editor);
    virtual ~ComputeDescriptorAction();

    void start(const ComputeDescriptorParameters &parameters);
    virtual void next();
    void clear();

    double minimum() const { return descriptorMinimum_; }
    double maximum() const { return descriptorMaximum_; }

protected:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    ComputeDescriptorParameters parameters_;
    ComputeDescriptorPca pca_;

    uint64_t numberOfPoints_;
    uint64_t numberOfPointsInFilter_;
    uint64_t numberOfPointsWithDescriptor_;

    double descriptorMinimum_;
    double descriptorMaximum_;

    void stepResetPoints();
    void stepCountPoints();
    void stepCompute();
    void stepNormalize();

    void computePoint();
};

#endif /* COMPUTE_DESCRIPTOR_ACTION_HPP */
