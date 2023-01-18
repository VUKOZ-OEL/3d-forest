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

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <SegmentationThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "SegmentationThread"

SegmentationThread::SegmentationThread(Editor *editor)
    : editor_(editor),
      query_(editor_)
{
    LOG_DEBUG(<< "Called.");
}

SegmentationThread::~SegmentationThread()
{
    LOG_DEBUG(<< "Called.");
}

void SegmentationThread::clear()
{
    query_.clear();
}

void SegmentationThread::restart(const SegmentationParameters &parameters)
{
    LOG_DEBUG(<< "Called.");

    // Cancel current computation
    cancel();

    Thread::start();
}

bool SegmentationThread::compute()
{
    LOG_DEBUG(<< "Compute.");

    bool threadFinished = true;

    if (callback_)
    {
        LOG_DEBUG(<< "Call callback with argument finished <" << threadFinished
                  << ">.");
        callback_->threadProgress(threadFinished);
    }

    return threadFinished;
}

int SegmentationThread::progressPercent() const
{
    return 100;
}
