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

/** @file DescriptorAlgorithmTaskNormalize.cpp */

#include <DescriptorAlgorithmTaskNormalize.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "DescriptorAlgorithmTaskNormalize"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void DescriptorAlgorithmTaskNormalize::initialize(
    DescriptorAlgorithmContext *context)
{
    context_ = context;
    context_->query.reset();

    ProgressActionInterface::initialize(context_->nPoints, 1000U);
}

void DescriptorAlgorithmTaskNormalize::next()
{
    uint64_t i = 0;

    startTimer();
    while (context_->query.next())
    {
        step();

        i++;
        if (timedOut())
        {
            increment(i);
            return;
        }
    }

    context_->query.flush();

    setProcessed(maximum());
}

void DescriptorAlgorithmTaskNormalize::step()
{
    double range = context_->descriptorMaximum - context_->descriptorMinimum;

    if (range > 0.0)
    {
        double v = context_->query.descriptor();
        if (v < std::numeric_limits<double>::max())
        {
            v = (v - context_->descriptorMinimum) / range;
        }
        else
        {
            v = 0;
        }

        context_->query.descriptor() = v;
    }
    else
    {
        context_->query.descriptor() = 0;
    }

    context_->query.setModified();
}
