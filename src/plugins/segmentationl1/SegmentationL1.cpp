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

/** @file SegmentationL1.cpp */

#include <Editor.hpp>
#include <SegmentationL1.hpp>

#define LOG_MODULE_NAME "SegmentationL1"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

SegmentationL1::SegmentationL1(Editor *editor) : context_(editor)
{
    LOG_DEBUG(<< "Create.");

    // Add individual actions from first to last.
    tasks_.push_back(&taskFilter_);
    tasks_.push_back(&taskSample_);
    tasks_.push_back(&taskPca_);
    tasks_.push_back(&taskMedian_);
    tasks_.push_back(&taskFinish_);

    clear();
}

SegmentationL1::~SegmentationL1()
{
    LOG_DEBUG(<< "Destroy.");
}

void SegmentationL1::clear()
{
    LOG_DEBUG(<< "Clear work data.");
    context_.clear();
    currentAction_ = SegmentationL1::npos;
}

bool SegmentationL1::applyParameters(const SegmentationL1Parameters &parameters)
{
    LOG_DEBUG(<< "Apply parameters <" << parameters << ">.");

    size_t newAction = SegmentationL1::npos;

    // Neighborhood Radius or the number of iterations has been changed.
    if ((context_.parameters.neighborhoodRadiusMinimum !=
         parameters.neighborhoodRadiusMinimum) ||
        (context_.parameters.neighborhoodRadiusMaximum !=
         parameters.neighborhoodRadiusMaximum) ||
        (context_.parameters.numberOfIterations !=
         parameters.numberOfIterations))
    {
        newAction = 3;
    }

    // Neighborhood PCA Radius has been changed.
    if (context_.parameters.neighborhoodRadiusPca !=
        parameters.neighborhoodRadiusPca)
    {
        newAction = 2;
    }

    // The number of initial samples has been changed.
    if (context_.parameters.numberOfSamples != parameters.numberOfSamples)
    {
        newAction = 1;
    }

    // Filter for the initial samples has been changed.
    if ((context_.parameters.sampleDescriptorMinimum !=
         parameters.sampleDescriptorMinimum) ||
        (context_.parameters.sampleDescriptorMaximum !=
         parameters.sampleDescriptorMaximum) ||
        (context_.nPoints == 0))
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

bool SegmentationL1::next()
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

void SegmentationL1::progress(size_t &nTasks,
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

void SegmentationL1::initializeCurrentAction()
{
    if (currentAction_ < tasks_.size())
    {
        tasks_[currentAction_]->initialize(&context_);
    }
}
