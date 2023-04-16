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

/** @file SegmentationL1TaskFilter.cpp */

#include <Editor.hpp>
#include <SegmentationL1TaskFilter.hpp>

#define LOG_MODULE_NAME "SegmentationL1TaskFilter"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void SegmentationL1TaskFilter::initialize(SegmentationL1Context *context)
{
    context_ = context;
    context_->nPoints = 0;
    context_->samples.resize(0);
    context_->samplesBackup = context_->samples;

    context_->execInitialSamplesQuery();

    ProgressActionInterface::initialize(context_->editor->datasets().nPoints(),
                                        1000U);
}

void SegmentationL1TaskFilter::next()
{
    uint64_t i = 0;

    startTimer();
    while (context_->query.next())
    {
        context_->nPoints++;

        i++;
        if (timedOut())
        {
            increment(i);
            return;
        }
    }

    setProcessed(maximum());
}
