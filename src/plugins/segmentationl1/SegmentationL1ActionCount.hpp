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

/** @file SegmentationL1ActionCount.hpp */

#ifndef SEGMENTATION_L1_ACTION_COUNT_HPP
#define SEGMENTATION_L1_ACTION_COUNT_HPP

#include <Editor.hpp>
#include <SegmentationL1ActionInterface.hpp>

/** Segmentation L1 Action Count. */
class SegmentationL1ActionCount : public SegmentationL1ActionInterface
{
public:
    virtual void initialize(SegmentationL1Context *context)
    {
        context_ = context;

        context_->reset();

        context_->query.setWhere(context_->editor->viewports().where());
        context_->query.exec();

        if (context_->query.next())
        {
            context_->totalSamplesCount++;
        }

        uint64_t max = context_->editor->datasets().nPoints(
            context_->query.where().dataset());

        ProgressActionInterface::initialize(max, 1000UL);
    }

    virtual void next()
    {
        startTimer();
        while (context_->query.next())
        {
            context_->totalSamplesCount++;
            if (timedOut())
            {
                return;
            }
        }

        setProcessed(maximum());
    }

private:
    SegmentationL1Context *context_;
};

#endif /* SEGMENTATION_L1_ACTION_COUNT_HPP */
