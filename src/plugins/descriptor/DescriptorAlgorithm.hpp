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

/** @file DescriptorAlgorithm.hpp */

#ifndef DESCRIPTOR_ALGORITHM_HPP
#define DESCRIPTOR_ALGORITHM_HPP

#include <DescriptorAlgorithmContext.hpp>
#include <DescriptorAlgorithmTaskCompute.hpp>
#include <DescriptorAlgorithmTaskCount.hpp>
#include <DescriptorAlgorithmTaskNormalize.hpp>
class Editor;

/** Descriptor Algorithm. */
class DescriptorAlgorithm
{
public:
    static const size_t npos = SIZE_MAX;

    DescriptorAlgorithm(Editor *editor);
    ~DescriptorAlgorithm();

    void clear();
    bool applyParameters(const DescriptorAlgorithmParameters &parameters);
    bool next();
    void progress(size_t &nTasks, size_t &iTask, double &percent) const;

    const DescriptorAlgorithmContext &context() const { return context_; }

private:
    DescriptorAlgorithmContext context_;

    DescriptorAlgorithmTaskCount taskCount_;
    DescriptorAlgorithmTaskCompute taskCompute_;
    DescriptorAlgorithmTaskNormalize taskNormalize_;

    std::vector<DescriptorAlgorithmTaskInterface *> tasks_;
    size_t currentAction_;

    void initializeCurrentAction();
};

#endif /* DESCRIPTOR_ALGORITHM_HPP */
