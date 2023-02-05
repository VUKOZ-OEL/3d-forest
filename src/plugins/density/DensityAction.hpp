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

/** @file DensityAction.hpp */

#ifndef DENSITY_ACTION_HPP
#define DENSITY_ACTION_HPP

#include <ProgressActionInterface.hpp>
#include <Query.hpp>

class Editor;

/** Density Action. */
class DensityAction : public ProgressActionInterface
{
public:
    DensityAction(Editor *editor);
    virtual ~DensityAction();

    void initialize(double radius);
    virtual void next();
    void clear();

    size_t minimum() const { return densityMinimum_; }
    size_t maximum() const { return densityMaximum_; }

protected:
    Editor *editor_;
    Query queryPoints_;
    Query queryPoint_;

    double radius_;

    enum Status
    {
        STATUS_NEW,
        STATUS_COMPUTE_DENSITY,
        STATUS_NORMALIZE_DENSITY,
        STATUS_FINISHED
    };

    Status status_;

    uint64_t nPointsTotal_;
    uint64_t nPointsOneHalf_;
    uint64_t nPointsDone_;

    size_t densityMinimum_;
    size_t densityMaximum_;

    void stepComputeDensity();
    void stepNormalizeDensity();

    void determineMaximum();
};

#endif /* DENSITY_ACTION_HPP */
