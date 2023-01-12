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

/** @file RenderThread.cpp */

#include <Editor.hpp>
#include <Log.hpp>
#include <RenderThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "RenderThread"

RenderThread::RenderThread(Editor *editor)
    : editor_(editor),
      viewportId_(0),
      initialized_(false)
{
}

void RenderThread::render(size_t viewportId, const Camera &camera)
{
    LOG_DEBUG(<< "Called with parameter viewportId <" << viewportId << ">.");
    LOG_TRACE(<< "Called with parameter viewportId <" << viewportId << ">.");
    cancel();

    viewportId_ = viewportId;
    camera_ = camera;
    initialized_ = false;

    Thread::start();
}

bool RenderThread::compute()
{
    LOG_DEBUG(<< "Computation is initialized <" << initialized_ << ">.");
    if (!initialized_)
    {
        editor_->lock();
        editor_->viewports().applyCamera(viewportId_, camera_);
        editor_->unlock();
        initialized_ = true;
        return false;
    }

    LOG_DEBUG(<< "Compute nextState.");
    double t1 = Time::realTime();
    bool finished;
    editor_->lock();
    finished = editor_->viewports().nextState();
    editor_->unlock();
    double t2 = Time::realTime();
    double msec = (t2 - t1) * 1000.;

    if (callback_)
    {
        LOG_DEBUG(<< "Call callback argument finished <" << finished << "> ms <"
                  << msec << ">.");
        callback_->threadProgress(finished);
    }

    if (msec < 20.)
    {
        Time::msleep(1);
    }

    return finished;
}
