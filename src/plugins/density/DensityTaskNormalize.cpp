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

/** @file DensityTaskNormalize.cpp */

#include <DensityTaskNormalize.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "DensityTaskNormalize"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void DensityTaskNormalize::initialize(DensityContext *context)
{
    context_ = context;
    context_->query.reset();

    ProgressActionInterface::initialize(context_->nPoints, 1000U);
}

void DensityTaskNormalize::next()
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

void DensityTaskNormalize::step()
{
    size_t densityRange = context_->densityMaximum - context_->densityMinimum;

    if (densityRange > 0)
    {
        double d = 1.0 / static_cast<double>(densityRange);
        size_t value = context_->query.value() - context_->densityMinimum;
        context_->query.descriptor() = static_cast<double>(value) * d;
    }
    else
    {
        context_->query.descriptor() = 0;
    }

    context_->query.setModified();
}
