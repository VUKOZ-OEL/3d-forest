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
#include <Log.hpp>

SegmentationL1::SegmentationL1(Editor *editor) : context_(editor)
{
    LOG_DEBUG(<< "Create.");

    actions_.push_back(&actionCount_);
    actions_.push_back(&actionRandom_);

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

    if ((context_.parameters.initialSamplesCount !=
         parameters.initialSamplesCount) ||
        (context_.parameters.initialSamplesDensityMinimum !=
         parameters.initialSamplesDensityMinimum) ||
        (context_.parameters.initialSamplesDensityMaximum !=
         parameters.initialSamplesDensityMaximum))
    {
        newAction = 0;
    }

    if (newAction < actions_.size())
    {
        currentAction_ = newAction;
        context_.parameters = parameters;
        initializeCurrentAction();
        return true;
    }

    return false;
}

bool SegmentationL1::step()
{
    LOG_DEBUG(<< "Compute the next step.");

    if (currentAction_ < actions_.size())
    {
        actions_[currentAction_]->step();
        if (actions_[currentAction_]->end())
        {
            // Move to the next action.
            currentAction_++;
            initializeCurrentAction();
        }
    }

    return currentAction_ < actions_.size();
}

int SegmentationL1::progressPercent() const
{
    if (currentAction_ < actions_.size())
    {
        return static_cast<int>(actions_[currentAction_]->percent());
    }

    return 100;
}

void SegmentationL1::initializeCurrentAction()
{
    if (currentAction_ < actions_.size())
    {
        actions_[currentAction_]->initialize(&context_);
    }
}
