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

/** @file DensityAlgorithm.hpp */

#ifndef DENSITY_ALGORITHM_HPP
#define DENSITY_ALGORITHM_HPP

#include <DensityContext.hpp>
#include <DensityTaskCompute.hpp>
#include <DensityTaskCount.hpp>
#include <DensityTaskNormalize.hpp>
class Editor;

/** Density Algorithm. */
class DensityAlgorithm
{
public:
    static const size_t npos = SIZE_MAX;

    DensityAlgorithm(Editor *editor);
    ~DensityAlgorithm();

    void clear();
    bool applyParameters(const DensityParameters &parameters);
    bool next();
    void progress(size_t &nTasks, size_t &iTask, double &percent) const;

    const DensityContext &context() const { return context_; }

private:
    DensityContext context_;

    DensityTaskCount taskCount_;
    DensityTaskCompute taskCompute_;
    DensityTaskNormalize taskNormalize_;

    std::vector<DensityTaskInterface *> tasks_;
    size_t currentAction_;

    void initializeCurrentAction();
};

#endif /* DENSITY_ALGORITHM_HPP */
