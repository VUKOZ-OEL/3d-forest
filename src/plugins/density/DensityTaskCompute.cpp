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

/** @file DensityTaskCompute.cpp */

#include <DensityTaskCompute.hpp>
#include <Editor.hpp>

#define LOG_MODULE_NAME "DensityTaskCompute"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

void DensityTaskCompute::initialize(DensityContext *context)
{
    context_ = context;
    context_->densityMinimum = 0;
    context_->densityMaximum = 0;
    context_->query.reset();

    ProgressActionInterface::initialize(context_->nPoints, 1000U);
}

void DensityTaskCompute::next()
{
    startTimer();

    while (context_->query.next())
    {
        step();
        increment(1);

        if (timedOut())
        {
            return;
        }
    }

    setProcessed(maximum());
}

void DensityTaskCompute::step()
{
    double r = static_cast<double>(context_->parameters.neighborhoodRadius);

    context_->queryPoint.where().setSphere(context_->query.x(),
                                           context_->query.y(),
                                           context_->query.z(),
                                           r);
    context_->queryPoint.exec();

    size_t nPointsSphere = 0;
    while (context_->queryPoint.next())
    {
        nPointsSphere++;
    }

    context_->query.value() = nPointsSphere;
    context_->query.setModified();

    if (processed() == 0)
    {
        context_->densityMinimum = nPointsSphere;
        context_->densityMaximum = nPointsSphere;
    }
    else
    {
        updateRange(nPointsSphere,
                    context_->densityMinimum,
                    context_->densityMaximum);
    }
}
