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

/** @file Descriptor.hpp */

#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include <DescriptorPca.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Descriptor. */
class Descriptor : public ProgressActionInterface
{
public:
    enum Method
    {
        METHOD_PCA = 0,
        METHOD_DISTRIBUTION = 1
    };

    Descriptor(Editor *editor);
    ~Descriptor();

    int start(double radius, double voxelSize, Method method);
    void step();
    void clear();

    float minimum() const { return descriptorMinimum_; }
    float maximum() const { return descriptorMaximum_; }

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

    int currentStep_;
    int numberOfSteps_;

    uint64_t nPointsTotal_;
    uint64_t nPointsPerStep_;
    uint64_t nPointsProcessed_;
    uint64_t nPointsWithDescriptor_;

    float descriptorMinimum_;
    float descriptorMaximum_;

    void stepComputeDescriptor();
    void stepNormalizeDescriptor();
};

#endif /* DESCRIPTOR_HPP */
