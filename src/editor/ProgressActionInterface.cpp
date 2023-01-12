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

/** @file ProgressActionInterface.cpp */

#include <Log.hpp>
#include <ProgressActionInterface.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "ProgressActionInterface"

ProgressActionInterface::ProgressActionInterface()
    : nElements_(0),
      nElementsProcessed_(0),
      interleave_(0),
      secondsPerStep_(0),
      timeBegin_(0),
      timeNow_(0),
      interleaveCounter_(0)
{
}

void ProgressActionInterface::initialize(uint64_t nElements,
                                         uint64_t interleave,
                                         double secondsPerStep)
{
    nElements_ = nElements;
    nElementsProcessed_ = 0;
    interleave_ = interleave;
    secondsPerStep_ = secondsPerStep;

    timeBegin_ = 0;
    timeNow_ = 0;
    interleaveCounter_ = 0;
}

void ProgressActionInterface::startTimer()
{
    timeBegin_ = Time::realTime();
}

bool ProgressActionInterface::timedOut()
{
    interleaveCounter_++;
    if (interleaveCounter_ >= interleave_)
    {
        interleaveCounter_ = 0;

        timeNow_ = Time::realTime();
        if (timeNow_ - timeBegin_ > secondsPerStep_)
        {
            return true; // timed out
        }
    }

    return false;
}

double ProgressActionInterface::percent() const
{
    double p;

    if (nElements_ == 0)
    {
        p = 100.0;
    }
    else if (nElements_ == npos)
    {
        p = 0.0;
    }
    else
    {
        p = 100.0 * (static_cast<double>(nElementsProcessed_) /
                     static_cast<double>(nElements_));
    }

    LOG_DEBUG(<< "Action progress <" << p << "> processed <"
              << nElementsProcessed_ << "> from <" << nElements_ << ">.");

    return p;
}

void ProgressActionInterface::increment(uint64_t nElementsProcessed)
{
    nElementsProcessed_ += nElementsProcessed;

    LOG_DEBUG(<< "Action processed <" << nElementsProcessed_ << "> from <"
              << nElements_ << ">.");
}
