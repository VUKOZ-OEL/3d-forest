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

/** @file DensityAlgorithm.cpp */

#include <DensityAlgorithm.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "DensityAlgorithm"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

DensityAlgorithm::DensityAlgorithm(Editor *editor) : context_(editor)
{
    LOG_DEBUG(<< "Create.");

    // Add individual actions from first to last.
    tasks_.push_back(&taskCount_);
    tasks_.push_back(&taskCompute_);
    tasks_.push_back(&taskNormalize_);

    clear();
}

DensityAlgorithm::~DensityAlgorithm()
{
    LOG_DEBUG(<< "Destroy.");
}

void DensityAlgorithm::clear()
{
    LOG_DEBUG(<< "Clear work data.");
    context_.clear();
    currentAction_ = DensityAlgorithm::npos;
}

bool DensityAlgorithm::applyParameters(const DensityParameters &parameters)
{
    LOG_DEBUG(<< "Apply parameters <" << parameters << ">.");

    size_t newAction = DensityAlgorithm::npos;

    // Neighborhood Radius has been changed.
    if (context_.parameters.neighborhoodRadius != parameters.neighborhoodRadius)
    {
        newAction = 0;
    }

    if (newAction < tasks_.size())
    {
        // Restart algorithm calculation from corresponding action.
        currentAction_ = newAction;
        context_.parameters = parameters;
        initializeCurrentAction();
        return true;
    }

    // Nothing changed.
    return false;
}

bool DensityAlgorithm::next()
{
    LOG_DEBUG(<< "Compute the next step.");

    if (currentAction_ < tasks_.size())
    {
        // Compute one step in the current action.
        tasks_[currentAction_]->next();

        // Check if the current action is finished.
        if (tasks_[currentAction_]->end())
        {
            // Yes, move to the next action.
            currentAction_++;
            initializeCurrentAction();
        }
    }

    return currentAction_ < tasks_.size();
}

void DensityAlgorithm::progress(size_t &nTasks,
                                size_t &iTask,
                                double &percent) const
{
    nTasks = tasks_.size();

    if (currentAction_ < tasks_.size())
    {
        iTask = currentAction_;
        percent = tasks_[currentAction_]->percent();
    }
    else
    {
        iTask = nTasks;
        percent = 100.0;
    }
}

void DensityAlgorithm::initializeCurrentAction()
{
    if (currentAction_ < tasks_.size())
    {
        tasks_[currentAction_]->initialize(&context_);
    }
}
