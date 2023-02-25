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

/** @file DescriptorAction.hpp */

#ifndef DESCRIPTOR_ACTION_HPP
#define DESCRIPTOR_ACTION_HPP

#include <DescriptorPca.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Descriptor Action. */
class DescriptorAction : public ProgressActionInterface
{
public:
    enum Method
    {
        METHOD_PCA = 0,
        METHOD_DISTRIBUTION = 1
    };

    DescriptorAction(Editor *editor);
    virtual ~DescriptorAction();

    void initialize(double radius, double voxelSize, Method method);
    virtual void next();
    void clear();

    double minimum() const { return descriptorMinimum_; }
    double maximum() const { return descriptorMaximum_; }

protected:
    Editor *editor_;
    Query queryPoints_;
    Query queryPoint_;
    DescriptorPca pca_;

    enum Status
    {
        STATUS_NEW,
        STATUS_COMPUTE_DESCRIPTOR,
        STATUS_NORMALIZE_DESCRIPTOR,
        STATUS_FINISHED
    };

    Status status_;

    double radius_;
    double voxelSize_;
    Method method_;

    uint64_t nPointsTotal_;
    uint64_t nPointsOneHalf_;
    uint64_t nPointsDone_;
    uint64_t nPointsWithDescriptor_;

    double descriptorMinimum_;
    double descriptorMaximum_;

    void stepComputeDescriptor();
    void stepNormalizeDescriptor();

    void determineMaximum();
};

#endif /* DESCRIPTOR_ACTION_HPP */
