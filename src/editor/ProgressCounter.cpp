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

/** @file ProgressCounter.cpp */

#include <ProgressCounter.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "ProgressCounter"
//#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ProgressCounter::ProgressCounter()
    : maximumSteps_(0),
      valueSteps_(0),
      maximumStep_(0),
      valueStep_(0),
      timeout_(0),
      timeBegin_(0),
      timeNow_(0),
      interleave_(0),
      interleaveCounter_(0)
{
}

void ProgressCounter::setMaximumStep(uint64_t n,
                                     uint64_t interleave,
                                     double timeout)
{
    maximumStep_ = n;
    valueStep_ = 0;

    timeout_ = timeout;
    timeBegin_ = 0;
    timeNow_ = 0;

    interleave_ = interleave;
    interleaveCounter_ = 0;
}

void ProgressCounter::setMaximumSteps(size_t n)
{
    maximumSteps_ = n;
    valueSteps_ = 0;
    percentSteps_.resize(0);
}

void ProgressCounter::setMaximumSteps(const std::vector<double> &percent)
{
    maximumSteps_ = percent.size();
    valueSteps_ = 0;
    percentSteps_ = percent;
}

void ProgressCounter::startTimer()
{
    timeBegin_ = Time::realTime();
}

bool ProgressCounter::timedOut()
{
    interleaveCounter_++;
    if (interleaveCounter_ >= interleave_)
    {
        interleaveCounter_ = 0;

        timeNow_ = Time::realTime();
        if (timeNow_ - timeBegin_ > timeout_)
        {
            return true; // timed out
        }
    }

    return false;
}

double ProgressCounter::percent() const
{
    double p;

    if (maximumSteps_ < 2)
    {
        // progress in one step
        p = percentStep();
    }
    else if (valueSteps_ >= maximumSteps_)
    {
        // all steps are finished
        p = 100.0;
    }
    else
    {
        // progress in multiple steps
        p = 0.0;
        double prest = 100.0;

        for (size_t i = 0; i <= valueSteps_; i++)
        {
            double pstep;

            if (i < percentSteps_.size())
            {
                pstep = percentSteps_[i];
            }
            else
            {
                size_t n = maximumSteps_ - i;
                pstep = prest / static_cast<double>(n);
            }

            if (i == valueSteps_)
            {
                pstep *= 0.01 * percentStep();
            }

            p += pstep;
            prest -= pstep;
        }
    }

    LOG_DEBUG(<< "Progress percent <" << p << "> processed <" << valueStep_
              << "> from <" << maximumStep_ << "> step <" << valueSteps_
              << "> of <" << maximumSteps_ << ">.");

    return p;
}

double ProgressCounter::percentStep() const
{
    if (maximumStep_ == 0)
    {
        return 100.0;
    }

    if (maximumStep_ == npos)
    {
        return 0.0;
    }

    return 100.0 * (static_cast<double>(valueStep_) /
                    static_cast<double>(maximumStep_));
}
