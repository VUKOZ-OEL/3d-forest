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

// Include 3D Forest.
#include <DescriptorParameters.hpp>
#include <DescriptorPca.hpp>
#include <ProgressActionInterface.hpp>
#include <Query.hpp>
class Editor;

/** Descriptor Action. */
class DescriptorAction : public ProgressActionInterface
{
public:
    DescriptorAction(Editor *editor);
    virtual ~DescriptorAction();

    void start(const DescriptorParameters &parameters);
    virtual void next();
    void clear();

    double minimum() const { return descriptorMinimum_; }
    double maximum() const { return descriptorMaximum_; }

protected:
    Editor *editor_;
    Query query_;
    Query queryPoint_;

    DescriptorParameters parameters_;
    DescriptorPca pca_;

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

#endif /* DESCRIPTOR_ACTION_HPP */
