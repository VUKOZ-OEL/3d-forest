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

/** @file SegmentationThread.cpp */

#include <Log.hpp>
#include <SegmentationThread.hpp>

#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("SegmentationThread", msg)

SegmentationThread::SegmentationThread() : initialized_(false)
{
    LOG_DEBUG_LOCAL("");
}

void SegmentationThread::setup()
{
    LOG_DEBUG_LOCAL("");
    cancel();
    initialized_ = false;
    Thread::start();
}

bool SegmentationThread::compute()
{
    LOG_DEBUG_LOCAL("initialized=" << initialized_);
    if (!initialized_)
    {
        initialized_ = true;
        return false;
    }

    LOG_DEBUG_LOCAL("nextState");

    return true;
}
